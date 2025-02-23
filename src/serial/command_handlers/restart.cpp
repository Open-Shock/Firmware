#include "serial/command_handlers/common.h"

#include <esp_system.h>

void _handleRestartCommand(std::string_view arg, bool isAutomated) {
  (void)arg;

  ::Serial.println("Restarting ESP...");
  esp_restart();
}

OpenShock::Serial::CommandGroup OpenShock::Serial::CommandHandlers::RestartHandler() {
  auto group = OpenShock::Serial::CommandGroup("restart"sv);

  auto& cmd = group.addCommand("Restart the board"sv, _handleRestartCommand);

  return group;
}
