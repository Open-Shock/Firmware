#include "config/EStopConfig.h"

#include "Chipset.h"
#include "Common.h"
#include "config/internal/utils.h"
#include "Logging.h"

const char* const TAG = "Config::EStopConfig";

using namespace OpenShock::Config;

EStopConfig::EStopConfig()
  : enabled(OpenShock::IsValidInputPin(OPENSHOCK_ESTOP_PIN))
  , gpioPin(static_cast<gpio_num_t>(OPENSHOCK_ESTOP_PIN))
  , latching(OPENSHOCK_ESTOP_LATCHING)
  , active(false)
{
}

EStopConfig::EStopConfig(bool enabled, gpio_num_t gpioPin, bool latching, bool active)
  : enabled(enabled)
  , gpioPin(gpioPin)
  , latching(latching)
  , active(active)
{
}

void EStopConfig::ToDefault()
{
  enabled  = OpenShock::IsValidInputPin(OPENSHOCK_ESTOP_PIN);
  gpioPin  = static_cast<gpio_num_t>(OPENSHOCK_ESTOP_PIN);
  latching = OPENSHOCK_ESTOP_LATCHING;
  active   = false;
}

bool EStopConfig::FromFlatbuffers(const Serialization::Configuration::EStopConfig* config)
{
  if (config == nullptr) {
    ToDefault();  // Set to default if config is null
    return true;
  }

  gpioPin = static_cast<gpio_num_t>(config->gpio_pin());

  if (OpenShock::IsValidInputPin(static_cast<int8_t>(gpioPin))) {
    enabled = config->enabled();
  } else {
    enabled = false;
  }

  latching = config->latching();

  active = config->active();

  return true;
}

flatbuffers::Offset<OpenShock::Serialization::Configuration::EStopConfig> EStopConfig::ToFlatbuffers(flatbuffers::FlatBufferBuilder& builder, bool withSensitiveData) const
{
  return Serialization::Configuration::CreateEStopConfig(builder, enabled, gpioPin, active, latching);
}

bool EStopConfig::FromJSON(const cJSON* json)
{
  if (json == nullptr) {
    ToDefault();  // Set to default if config is null
    return true;
  }

  if (cJSON_IsObject(json) == 0) {
    OS_LOGE(TAG, "json is not an object");
    return false;
  }

  Internal::Utils::FromJsonGpioNum(gpioPin, json, "gpioPin", static_cast<gpio_num_t>(OPENSHOCK_ESTOP_PIN));

  if (!Internal::Utils::FromJsonBool(enabled, json, "enabled", OpenShock::IsValidInputPin(gpioPin))) {
    OS_LOGE(TAG, "Failed to parse enabled");
    return false;
  }

  if (!Internal::Utils::FromJsonBool(latching, json, "latching", OPENSHOCK_ESTOP_LATCHING)) {
    OS_LOGE(TAG, "Failed to parse latching");
    return false;
  }

  if (!Internal::Utils::FromJsonBool(active, json, "active", false)) {
    OS_LOGE(TAG, "Failed to parse active");
    return false;
  }

  return true;
}

cJSON* EStopConfig::ToJSON(bool withSensitiveData) const
{
  cJSON* root = cJSON_CreateObject();

  cJSON_AddBoolToObject(root, "enabled", enabled);
  cJSON_AddNumberToObject(root, "gpioPin", gpioPin);
  cJSON_AddBoolToObject(root, "latching", latching);
  cJSON_AddBoolToObject(root, "active", active);

  return root;
}
