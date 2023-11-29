#include "config/Config.h"

#include "config/RootConfig.h"
#include "Constants.h"
#include "Logging.h"

#include <LittleFS.h>

#include <cJSON.h>

#include <bitset>

const char* const TAG = "Config";

using namespace OpenShock;

Config::RootConfig _mainConfig;

bool _tryLoadConfig() {
  File file = LittleFS.open("/config", "rb");
  if (!file) {
    ESP_LOGE(TAG, "Failed to open config file for reading");
    return false;
  }

  // Get file size
  std::size_t size = file.size();

  // Allocate buffer
  std::vector<std::uint8_t> buffer(size);

  // Read file
  if (file.read(buffer.data(), buffer.size()) != buffer.size()) {
    ESP_LOGE(TAG, "Failed to read config file, size mismatch");
    return false;
  }

  file.close();

  // Deserialize
  auto fbsConfig = flatbuffers::GetRoot<Serialization::Configuration::Config>(buffer.data());
  if (fbsConfig == nullptr) {
    ESP_LOGE(TAG, "Failed to get deserialization root for config file");
    return false;
  }

  // Validate buffer
  flatbuffers::Verifier::Options verifierOptions {
    .max_size = 4096,  // Should be enough
  };
  flatbuffers::Verifier verifier(buffer.data(), buffer.size(), verifierOptions);
  if (!fbsConfig->Verify(verifier)) {
    ESP_LOGE(TAG, "Failed to verify config file integrity");
    return false;
  }

  // Read config
  if (!_mainConfig.FromFlatbuffers(fbsConfig)) {
    ESP_LOGE(TAG, "Failed to read config file");
    return false;
  }

  return true;
}
bool _trySaveConfig() {
  File file = LittleFS.open("/config", "wb");
  if (!file) {
    ESP_LOGE(TAG, "Failed to open config file for writing");
    return false;
  }

  auto& _rf            = _mainConfig.rf;
  auto& _wifi          = _mainConfig.wifi;
  auto& _backend       = _mainConfig.backend;
  auto& _captivePortal = _mainConfig.captivePortal;

  // Serialize
  flatbuffers::FlatBufferBuilder builder(1024);

  builder.Finish(_mainConfig.ToFlatbuffers(builder));

  // Write file
  if (file.write(builder.GetBufferPointer(), builder.GetSize()) != builder.GetSize()) {
    ESP_LOGE(TAG, "Failed to write config file");
    return false;
  }

  file.close();

  return true;
}

void Config::Init() {
  if (_tryLoadConfig()) {
    return;
  }

  ESP_LOGW(TAG, "Failed to load config, writing default config");

  _mainConfig.ToDefault();

  if (!_trySaveConfig()) {
    ESP_PANIC(TAG, "Failed to save default config. Recommend formatting microcontroller and re-flashing firmware");
  }
}

std::string Config::GetAsJSON() {
  cJSON* root = _mainConfig.ToJSON();

  char* json = cJSON_PrintUnformatted(root);

  std::string result(json);

  free(json);

  cJSON_Delete(root);

  return result;
}
bool Config::SaveFromJSON(const std::string& json) {
  cJSON* root = cJSON_Parse(json.c_str());
  if (root == nullptr) {
    ESP_LOGE(TAG, "Failed to parse JSON: %s", cJSON_GetErrorPtr());
    return false;
  }

  bool result = _mainConfig.FromJSON(root);

  cJSON_Delete(root);

  if (!result) {
    ESP_LOGE(TAG, "Failed to read JSON");
    return false;
  }

  return _trySaveConfig();
}

void Config::FactoryReset() {
  if (!LittleFS.remove("/config") && LittleFS.exists("/config")) {
    ESP_PANIC(TAG, "Failed to remove existing config file for factory reset. Reccomend formatting microcontroller and re-flashing firmware");
  }
}

const Config::RFConfig& Config::GetRFConfig() {
  return _mainConfig.rf;
}

const Config::WiFiConfig& Config::GetWiFiConfig() {
  return _mainConfig.wifi;
}

const std::vector<Config::WiFiCredentials>& Config::GetWiFiCredentials() {
  return _mainConfig.wifi.credentialsList;
}

const Config::CaptivePortalConfig& Config::GetCaptivePortalConfig() {
  return _mainConfig.captivePortal;
}

const Config::BackendConfig& Config::GetBackendConfig() {
  return _mainConfig.backend;
}

bool Config::SetRFConfig(const Config::RFConfig& config) {
  _mainConfig.rf = config;
  return _trySaveConfig();
}

bool Config::SetWiFiConfig(const Config::WiFiConfig& config) {
  _mainConfig.wifi = config;
  return _trySaveConfig();
}

bool Config::SetWiFiCredentials(const std::vector<Config::WiFiCredentials>& credentials) {
  for (auto& cred : credentials) {
    if (cred.id == 0) {
      ESP_LOGE(TAG, "Cannot set WiFi credentials: credential ID cannot be 0");
      return false;
    }
  }

  _mainConfig.wifi.credentialsList = credentials;
  return _trySaveConfig();
}

bool Config::SetCaptivePortalConfig(const Config::CaptivePortalConfig& config) {
  _mainConfig.captivePortal = config;
  return _trySaveConfig();
}

bool Config::SetBackendConfig(const Config::BackendConfig& config) {
  _mainConfig.backend = config;
  return _trySaveConfig();
}

bool Config::SetRFConfigTxPin(std::uint8_t txPin) {
  _mainConfig.rf.txPin = txPin;
  return _trySaveConfig();
}

bool Config::SetRFConfigKeepAliveEnabled(bool enabled) {
  _mainConfig.rf.keepAliveEnabled = enabled;
  return _trySaveConfig();
}

std::uint8_t Config::AddWiFiCredentials(const std::string& ssid, const std::string& password) {
  std::uint8_t id = 0;

  std::bitset<255> bits;
  for (auto it = _mainConfig.wifi.credentialsList.begin(); it != _mainConfig.wifi.credentialsList.end(); ++it) {
    auto& creds = *it;

    if (creds.ssid == ssid) {
      creds.password = password;

      id = creds.id;

      break;
    }

    if (creds.id == 0) {
      ESP_LOGW(TAG, "Found WiFi credentials with ID 0, removing");
      it = _mainConfig.wifi.credentialsList.erase(it);
      continue;
    }

    // Mark ID as used
    bits[creds.id - 1] = true;
  }

  // Get first available ID
  for (std::size_t i = 0; i < bits.size(); ++i) {
    if (!bits[i]) {
      id = i + 1;
      break;
    }
  }

  if (id == 0) {
    ESP_LOGE(TAG, "Failed to add WiFi credentials: no available IDs");
    return 0;
  }

  _mainConfig.wifi.credentialsList.push_back({
    .id       = id,
    .ssid     = ssid,
    .password = password,
  });
  _trySaveConfig();

  return id;
}

bool Config::TryGetWiFiCredentialsByID(std::uint8_t id, Config::WiFiCredentials& credentials) {
  for (auto& creds : _mainConfig.wifi.credentialsList) {
    if (creds.id == id) {
      credentials = creds;
      return true;
    }
  }

  return false;
}

bool Config::TryGetWiFiCredentialsBySSID(const char* ssid, Config::WiFiCredentials& credentials) {
  for (auto& creds : _mainConfig.wifi.credentialsList) {
    if (creds.ssid == ssid) {
      credentials = creds;
      return true;
    }
  }

  return false;
}

std::uint8_t Config::GetWiFiCredentialsIDbySSID(const char* ssid) {
  for (auto& creds : _mainConfig.wifi.credentialsList) {
    if (creds.ssid == ssid) {
      return creds.id;
    }
  }

  return 0;
}

bool Config::RemoveWiFiCredentials(std::uint8_t id) {
  for (auto it = _mainConfig.wifi.credentialsList.begin(); it != _mainConfig.wifi.credentialsList.end(); ++it) {
    if (it->id == id) {
      _mainConfig.wifi.credentialsList.erase(it);
      _trySaveConfig();
      return true;
    }
  }

  return false;
}

void Config::ClearWiFiCredentials() {
  _mainConfig.wifi.credentialsList.clear();
  _trySaveConfig();
}

bool Config::HasBackendAuthToken() {
  return !_mainConfig.backend.authToken.empty();
}

const std::string& Config::GetBackendAuthToken() {
  return _mainConfig.backend.authToken;
}

bool Config::SetBackendAuthToken(const std::string& token) {
  _mainConfig.backend.authToken = token;
  return _trySaveConfig();
}

bool Config::ClearBackendAuthToken() {
  _mainConfig.backend.authToken = "";
  return _trySaveConfig();
}
