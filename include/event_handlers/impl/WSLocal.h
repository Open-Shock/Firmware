#pragma once

#include "serialization/_fbs/DeviceToLocalMessage_generated.h"
#include "serialization/_fbs/LocalToDeviceMessage_generated.h"

#include <cstdint>

#define _HANDLER_SIGNATURE(NAME) void NAME(std::uint8_t socketId, const OpenShock::Serialization::Local::LocalToDeviceMessage* msg)

namespace OpenShock::MessageHandlers::Local::_Private {
  typedef _HANDLER_SIGNATURE((*HandlerType));
  _HANDLER_SIGNATURE(HandleInvalidMessage);
  _HANDLER_SIGNATURE(HandleWiFiScanCommand);
  _HANDLER_SIGNATURE(HandleWiFiNetworkSaveCommand);
  _HANDLER_SIGNATURE(HandleWiFiNetworkForgetCommand);
  _HANDLER_SIGNATURE(HandleWiFiNetworkConnectCommand);
  _HANDLER_SIGNATURE(HandleWiFiNetworkDisconnectCommand);
  _HANDLER_SIGNATURE(HandleAccountLinkCommand);
  _HANDLER_SIGNATURE(HandleAccountUnlinkCommand);
  _HANDLER_SIGNATURE(HandleSetRfTxPinCommand);
}  // namespace OpenShock::MessageHandlers::Local::_Private
