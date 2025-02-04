#include "serial/command_handlers/CommandEntry.h"

using namespace OpenShock::Serial;

CommandEntry::CommandEntry(std::string_view description, CommandHandler commandHandler)
  : m_description(description)
  , m_commandHandler(commandHandler)
{
}

CommandEntry::CommandEntry(std::string_view name, std::string_view description, CommandHandler commandHandler)
  : m_name(name)
  , m_description(description)
  , m_commandHandler(commandHandler)
{
}

CommandArgument& CommandEntry::addArgument(std::string_view name, std::string_view constraint, std::string_view exampleValue, std::vector<std::string_view> constraintExtensions)
{
  m_arguments.push_back({name, constraint, exampleValue, constraintExtensions});
  return m_arguments.back();
}
