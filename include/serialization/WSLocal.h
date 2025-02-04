#pragma once

#include "serialization/CallbackFn.h"
#include "wifi/WiFiScanStatus.h"

#include "serialization/_fbs/WifiNetworkEventType_generated.h"

#include <esp_wifi_types.h>

#include <string_view>

namespace OpenShock {
  class WiFiNetwork;
}

namespace OpenShock::Serialization::Local {
  bool SerializeErrorMessage(std::string_view message, Common::SerializationCallbackFn callback);
  bool SerializeReadyMessage(const WiFiNetwork* connectedNetwork, bool accountLinked, Common::SerializationCallbackFn callback);
  bool SerializeWiFiScanStatusChangedEvent(OpenShock::WiFiScanStatus status, Common::SerializationCallbackFn callback);
  bool SerializeWiFiNetworkEvent(Types::WifiNetworkEventType eventType, const WiFiNetwork& network, Common::SerializationCallbackFn callback);
  bool SerializeWiFiNetworksEvent(Types::WifiNetworkEventType eventType, const std::vector<WiFiNetwork>& networks, Common::SerializationCallbackFn callback);
}  // namespace OpenShock::Serialization::Local
