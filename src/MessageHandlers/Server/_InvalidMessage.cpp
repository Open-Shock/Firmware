#include "MessageHandlers/Server_Private.h"

#include "Logging.h"

const char* const TAG = "ServerMessageHandlers";

using namespace OpenShock::MessageHandlers::Server;

void _Private::HandleInvalidMessage(const OpenShock::Serialization::ServerToDeviceMessage* root) {
  if (root == nullptr) {
    ESP_LOGE(TAG, "Message cannot be parsed");
    return;
  }

  ESP_LOGE(TAG, "Invalid message type: %d", root->payload_type());
}
