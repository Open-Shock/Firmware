#include "serial/SerialInputHandler.h"

const char* const TAG = "SerialInputHandler";

#include "Chipset.h"
#include "CommandHandler.h"
#include "config/Config.h"
#include "config/SerialInputConfig.h"
#include "Convert.h"
#include "estop/EStopManager.h"
#include "FormatHelpers.h"
#include "http/HTTPRequestManager.h"
#include "Logging.h"
#include "serial/command_handlers/CommandEntry.h"
#include "serial/command_handlers/common.h"
#include "serial/command_handlers/index.h"
#include "serial/SerialBuffer.h"
#include "serial/SerialReadResult.h"
#include "serialization/JsonAPI.h"
#include "serialization/JsonSerial.h"
#include "Time.h"
#include "util/Base64Utils.h"
#include "util/StringUtils.h"
#include "util/TaskUtils.h"
#include "wifi/WiFiManager.h"

#include <Arduino.h>

#include <cJSON.h>
#include <Esp.h>

#include <cstring>
#include <string_view>
#include <unordered_map>

namespace std {
  struct hash_ci {
    std::size_t operator()(std::string_view str) const
    {
      std::size_t hash = 7;

      for (int i = 0; i < str.size(); ++i) {
        hash = hash * 31 + tolower(str[i]);
      }

      return hash;
    }
  };

  template<>
  struct less<std::string_view> {
    bool operator()(std::string_view a, std::string_view b) const { return a < b; }
  };

  struct equals_ci {
    bool operator()(std::string_view a, std::string_view b) const { return OpenShock::StringIEquals(a, b); }
  };
}  // namespace std

#define OPENSHOCK_WELCOME_HEADER_STR \
  "\n\
============== OPENSHOCK ==============\n\
  Contribute @ github.com/OpenShock\n\
  Discuss    @ discord.gg/OpenShock\n\
  Type 'help' for available commands\n\
=======================================\n"

#define OPENSHOCK_VERSION_INFO_STR \
  "\
  Version:  " OPENSHOCK_FW_VERSION "\n\
    Build:  " OPENSHOCK_FW_MODE "\n\
   Commit:  " OPENSHOCK_FW_GIT_COMMIT "\n\
    Board:  " OPENSHOCK_FW_BOARD "\n\
     Chip:  " OPENSHOCK_FW_CHIP "\n"

#define CLEAR_LINE "\r\x1B[K"

const int64_t PASTE_INTERVAL_THRESHOLD_MS       = 20;
const std::size_t SERIAL_BUFFER_CLEAR_THRESHOLD = 512;

static bool serialEchoEnabled = true;
static std::vector<OpenShock::Serial::CommandGroup> serialCommandGroups;
static std::unordered_map<std::string_view, OpenShock::Serial::CommandGroup, std::hash_ci, std::equals_ci> serialCommandHandlers;

using namespace OpenShock;
using namespace std::string_view_literals;

static void printCommandsHelp()
{
  std::size_t commandCount    = 0;
  std::size_t longestCommand  = 0;
  std::size_t longestArgument = 0;
  std::size_t descriptionSize = 0;
  for (const auto& group : serialCommandGroups) {
    longestCommand = std::max(longestCommand, group.name().size());
    for (const auto& command : group.commands()) {
      commandCount++;

      std::size_t argumentSize = 0;
      if (command.name().size() > 0) {
        argumentSize += command.name().size() + 1;  // +1 for space
      }
      for (const auto& arg : command.arguments()) {
        argumentSize += arg.name.size() + 3;  // +1 for space, +2 for <>
      }
      longestArgument = std::max(longestArgument, argumentSize);
      descriptionSize += command.description().size();
    }
  }

  std::size_t paddedLength = longestCommand + 1 + longestArgument + 1;  // +1 for space, +1 for newline

  std::string buffer;
  buffer.reserve((paddedLength * commandCount) + descriptionSize);  // Approximate size

  for (const auto& group : serialCommandGroups) {
    for (const auto& command : group.commands()) {
      buffer.append(group.name());
      buffer.append((longestCommand - group.name().size()) + 1, ' ');

      std::size_t startSize = buffer.size();

      if (command.name().size() > 0) {
        buffer.append(command.name());
        buffer.push_back(' ');
      }

      for (const auto& arg : command.arguments()) {
        buffer.push_back('<');
        buffer.append(arg.name);
        buffer.push_back('>');
        buffer.push_back(' ');
      }

      buffer.append(longestArgument - (buffer.size() - startSize), ' ');

      buffer.append(command.description());

      buffer.push_back('\n');
    }
  }

  SerialInputHandler::PrintBootInfo();
  ::Serial.print(buffer.data());
}

static void printCommandHelp(Serial::CommandGroup& group)
{
  std::size_t size = 0;
  for (const auto& command : group.commands()) {
    size++;  // +1 for newline
    size += group.name().size();
    size++;  // +1 for space

    if (command.name().size() > 0) {
      size += command.name().size() + 1;  // +1 for space
    }

    for (const auto& arg : command.arguments()) {
      size += arg.name.size() + 3;  // +1 for space, +2 for <>
    }

    size++;  // +1 for newline

    if (command.description().size() > 0) {
      size = command.description().size() + 3;  // +2 for indent, +1 for newline
    }

    if (command.arguments().size() > 0) {
      size += 13;                     // +13 for "  Arguments:\n"
      for (const auto& arg : command.arguments()) {
        size += arg.name.size() + 7;  // +4 for indent, +2 for <>, +1 for space
        size += arg.constraint.size();
        if (arg.constraintExtensions.size() > 0) {
          size += 2;                 // +1 for ':', +1 for newline
          for (const auto& ext : arg.constraintExtensions) {
            size += ext.size() + 7;  // +1 for newline, +6 for indent
          }
        } else {
          size++;  // +1 for newline
        }
      }
    }

    size += 15;                       // +15 for "  Example:    \n"
    size += group.name().size() + 1;  // +1 for space

    if (command.name().size() > 0) {
      size += command.name().size() + 1;  // +1 for space
    }

    for (const auto& arg : command.arguments()) {
      size += arg.exampleValue.size() + 1;  // +1 for space
    }

    size++;  // +1 for newline
  }

  size++;  // +1 for newline

  std::string buffer;
  buffer.reserve(size);  // TODO: Should be exact size, is 20 bytes off, figure out why

  for (const auto& command : group.commands()) {
    buffer.push_back('\n');
    buffer.append(group.name());
    buffer.push_back(' ');

    if (command.name().size() > 0) {
      buffer.append(command.name());
      buffer.push_back(' ');
    }

    for (const auto& arg : command.arguments()) {
      buffer.push_back('<');
      buffer.append(arg.name);
      buffer.push_back('>');
      buffer.push_back(' ');
    }

    buffer.push_back('\n');

    if (command.description().size() > 0) {
      buffer.append(2, ' ');
      buffer.append(command.description());
      buffer.push_back('\n');
    }

    if (command.arguments().size() > 0) {
      buffer.append("  Arguments:\n"sv);
      for (const auto& arg : command.arguments()) {
        buffer.append(4, ' ');
        buffer.push_back('<');
        buffer.append(arg.name);
        buffer.push_back('>');
        buffer.push_back(' ');
        buffer.append(arg.constraint);
        if (arg.constraintExtensions.size() > 0) {
          buffer.push_back('\n');
          for (const auto& ext : arg.constraintExtensions) {
            buffer.append(6, ' ');
            buffer.append(ext);
            buffer.push_back('\n');
          }
        } else {
          buffer.push_back('\n');
        }
      }
    }

    buffer.append("  Example:\n    "sv);
    buffer.append(group.name());
    buffer.push_back(' ');

    if (command.name().size() > 0) {
      buffer.append(command.name());
      buffer.push_back(' ');
    }

    for (const auto& arg : command.arguments()) {
      buffer.append(arg.exampleValue);
      buffer.push_back(' ');
    }

    buffer.push_back('\n');
  }
  buffer.push_back('\n');

  ::Serial.print(buffer.data());
}

static void handleHelpCommand(std::string_view arg, bool isAutomated)
{
  arg = OpenShock::StringTrim(arg);
  if (arg.empty()) {
    printCommandsHelp();
    return;
  }

  // Get help for a specific command
  auto it = serialCommandHandlers.find(arg);
  if (it != serialCommandHandlers.end()) {
    printCommandHelp(it->second);
    return;
  }

  SERPR_ERROR("Command \"%.*s\" not found", arg.length(), arg.data());
}

static void registerCommandHandler(const OpenShock::Serial::CommandGroup& handler)
{
  serialCommandHandlers[handler.name()] = handler;
}

static OpenShock::Serial::SerialReadResult serialTryReadLine(OpenShock::Serial::SerialBuffer& buffer)
{
  // Check if there's any data available
  int available = ::Serial.available();
  if (available <= 0) {
    return OpenShock::Serial::SerialReadResult::NoData;
  }

  // Reserve space for the new data
  buffer.reserve(buffer.size() + available);

  // Read the data into the buffer
  while (available-- > 0) {
    char c = ::Serial.read();

    // Handle backspace
    if (c == '\b') {
      buffer.pop_back();  // Remove the last character from the buffer if it exists
      continue;
    }

    // Handle newline
    if (c == '\r' || c == '\n') {
      if (!buffer.empty()) {
        return OpenShock::Serial::SerialReadResult::LineEnd;
      }
      continue;
    }

    // Handle leading whitespace
    if (c == ' ' && buffer.empty()) {
      continue;
    }

    if (c == '\t') {
      return OpenShock::Serial::SerialReadResult::AutoCompleteRequest;
    }

    // If character is printable, add it to the buffer
    if (c > 31 && c < 127) {
      buffer.push_back(c);
    }
  }

  return OpenShock::Serial::SerialReadResult::Data;
}

static void serialSkipWhitespaces(OpenShock::Serial::SerialBuffer& buffer)
{
  int available = ::Serial.available();

  while (available-- > 0) {
    char c = ::Serial.read();

    if (c != ' ' && c != '\r' && c != '\n') {
      buffer.push_back(c);
      break;
    }
  }
}

static void serialEchoBuffer(std::string_view buffer)
{
  ::Serial.printf(CLEAR_LINE "> %.*s", buffer.size(), buffer.data());
  ::Serial.flush();
}

static void serialHandleActivity(std::string_view buffer, bool hasData)
{
  static int64_t lastActivity = 0;
  static bool hasChanges      = false;

  // If serial echo is disabled, don't do anything past this point
  if (!serialEchoEnabled) {
    return;
  }

  // If the command starts with a $, it's a automated command, don't echo it
  if (OpenShock::StringHasPrefix(buffer, '$')) {
    return;
  }

  // Update activity state
  if (hasData) {
    hasChanges   = true;
    lastActivity = OpenShock::millis();
  }

  // If theres has been received data, but no new data for a while, echo the buffer
  if (hasChanges && OpenShock::millis() - lastActivity > PASTE_INTERVAL_THRESHOLD_MS) {
    serialEchoBuffer(buffer);
    hasChanges   = false;
    lastActivity = OpenShock::millis();
  }
}

static const OpenShock::Serial::CommandEntry* getCommandEntry(const std::vector<std::string_view>& arguments, const std::vector<OpenShock::Serial::CommandEntry>& commandsEntries)
{
  if (arguments.empty()) {
    // If no arguments, check for the first command with no name and no arguments
    for (const auto& command : commandsEntries) {
      if (command.name().empty() && command.arguments().empty()) {
        return &command;
      }
    }
    return nullptr;
  }

  const OpenShock::Serial::CommandEntry* bestMatch = nullptr;
  size_t bestMatchArgsCount                        = 0;

  for (const auto& command : commandsEntries) {
    auto commandName = command.name();
    auto commandArgs = command.arguments();

    if (commandName.empty()) {
      // Unnamed commands: select if argument size fits and has the closest to that many arguments seen yet
      if (commandArgs.size() <= arguments.size() && commandArgs.size() >= bestMatchArgsCount) {
        bestMatch          = &command;
        bestMatchArgsCount = commandArgs.size();
      }
    } else {
      // Named commands: exact name match, argument size fits and has the closest to that many arguments seen yet
      if (commandName == arguments[0] && commandArgs.size() < arguments.size() && commandArgs.size() >= bestMatchArgsCount) {
        bestMatch          = &command;
        bestMatchArgsCount = commandArgs.size();
      }
    }
  }

  return bestMatch;
}

static void serialProcessLine(std::string_view line)
{
  line = OpenShock::StringTrim(line);
  if (line.empty()) {
    return;
  }

  bool isAutomated = line[0] == '$';

  // If automated, remove the $ prefix
  // If it's not automated, we can echo the command if echo is enabled
  if (isAutomated) {
    line = line.substr(1);
  } else if (serialEchoEnabled) {
    serialEchoBuffer(line);
  }

  auto splitArguments        = OpenShock::StringSplit(line, ' ', 1);
  std::string_view command   = OpenShock::StringTrim(splitArguments[0]);
  std::string_view arguments = splitArguments.size() > 1 ? splitArguments[1] : std::string_view();

  if (command == "help"sv) {
    handleHelpCommand(arguments, isAutomated);
    return;
  }

  auto it = serialCommandHandlers.find(command);
  if (it == serialCommandHandlers.end()) {
    SERPR_ERROR("Command \"%.*s\" not found", command.size(), command.data());
    return;
  }

  auto commandEntry = getCommandEntry(OpenShock::StringSplit(arguments, ' '), it->second.commands());
  if (commandEntry == nullptr) {
    printCommandHelp(it->second);
    return;
  }

  auto commandEntryName = commandEntry->name();
  if (!commandEntryName.empty()) {
    arguments = OpenShock::StringTrim(arguments.substr(commandEntryName.size()));
  }

  commandEntry->commandHandler()(arguments, isAutomated);
}

static void serialTaskRX(void*)
{
  OpenShock::Serial::SerialBuffer buffer(32);

  while (true) {
    switch (serialTryReadLine(buffer)) {
      case OpenShock::Serial::SerialReadResult::LineEnd:
        serialProcessLine(buffer);

        // Deallocate memory if the buffer is too large
        if (buffer.capacity() > SERIAL_BUFFER_CLEAR_THRESHOLD) {
          buffer.destroy();
        } else {
          buffer.clear();
        }

        // Skip any remaining trailing whitespaces
        serialSkipWhitespaces(buffer);
        break;
      case OpenShock::Serial::SerialReadResult::AutoCompleteRequest:
        ::Serial.printf(CLEAR_LINE "> %.*s [AutoComplete is not implemented]", buffer.size(), buffer.data());
        break;
      case OpenShock::Serial::SerialReadResult::Data:
        serialHandleActivity(buffer, true);
        break;
      default:
        serialHandleActivity(buffer, false);
        break;
    }

    vTaskDelay(pdMS_TO_TICKS(20));  // 50 Hz update rate
  }
}

bool SerialInputHandler::Init()
{
  static bool s_initialized = false;
  if (s_initialized) {
    OS_LOGW(TAG, "Serial input handler already initialized");
    return false;
  }
  s_initialized = true;

  // Register command handlers
  serialCommandGroups = OpenShock::Serial::CommandHandlers::AllCommandHandlers();
  for (const auto& handler : serialCommandGroups) {
    OS_LOGV(TAG, "Registering command handler: %.*s", handler.name().size(), handler.name().data());
    registerCommandHandler(handler);
  }

  SerialInputHandler::PrintBootInfo();

  if (!Config::GetSerialInputConfigEchoEnabled(serialEchoEnabled)) {
    OS_LOGE(TAG, "Failed to get serial echo status from config");
    return false;
  }

  if (TaskUtils::TaskCreateExpensive(serialTaskRX, "SerialRX", 10'000, nullptr, 1, nullptr) != pdPASS) {  // TODO: Profile stack size
    OS_LOGE(TAG, "Failed to create serial RX task");
    return false;
  }

  return true;
}
bool SerialInputHandler::SerialEchoEnabled()
{
  return serialEchoEnabled;
}

void SerialInputHandler::SetSerialEchoEnabled(bool enabled)
{
  serialEchoEnabled = enabled;
}

void SerialInputHandler::PrintBootInfo()
{
  ::Serial.print(OpenShock::StringToArduinoString(OPENSHOCK_WELCOME_HEADER_STR OPENSHOCK_VERSION_INFO_STR "\n"sv));
}

void SerialInputHandler::PrintVersionInfo()
{
  ::Serial.print(OpenShock::StringToArduinoString("\n" OPENSHOCK_VERSION_INFO_STR ""sv));
}
