#include "config/BackendConfig.h"

#include "config/internal/utils.h"
#include "Logging.h"

const char* const TAG = "Config::BackendConfig";

using namespace OpenShock::Config;

BackendConfig::BackendConfig() : domain(OPENSHOCK_API_DOMAIN), authToken() { }

void BackendConfig::ToDefault() {
  domain = OPENSHOCK_API_DOMAIN;
  authToken.clear();
}

bool BackendConfig::FromFlatbuffers(const Serialization::Configuration::BackendConfig* config) {
  if (config == nullptr) {
    ESP_LOGE(TAG, "config is null");
    return false;
  }

  Internal::Utils::FromFbsStr(domain, config->domain(), OPENSHOCK_API_DOMAIN);
  Internal::Utils::FromFbsStr(authToken, config->auth_token(), "");

  return true;
}

flatbuffers::Offset<OpenShock::Serialization::Configuration::BackendConfig> BackendConfig::ToFlatbuffers(flatbuffers::FlatBufferBuilder& builder, bool withSensitiveData) const {
  auto domainOffset = builder.CreateString(domain);

  flatbuffers::Offset<flatbuffers::String> authTokenOffset;
  if (withSensitiveData) {
    authTokenOffset = builder.CreateString(authToken);
  } else {
    authTokenOffset = 0;
  }

  return Serialization::Configuration::CreateBackendConfig(builder, domainOffset, authTokenOffset);
}

bool BackendConfig::FromJSON(const cJSON* json) {
  if (json == nullptr) {
    ESP_LOGE(TAG, "json is null");
    return false;
  }

  if (cJSON_IsObject(json) == 0) {
    ESP_LOGE(TAG, "json is not an object");
    return false;
  }

  const cJSON* domainJson = cJSON_GetObjectItemCaseSensitive(json, "domain");
  if (domainJson == nullptr) {
    ESP_LOGE(TAG, "domain is null");
    return false;
  }

  if (cJSON_IsString(domainJson) == 0) {
    ESP_LOGE(TAG, "domain is not a string");
    return false;
  }

  domain = domainJson->valuestring;

  const cJSON* authTokenJson = cJSON_GetObjectItemCaseSensitive(json, "authToken");
  if (authTokenJson == nullptr) {
    ESP_LOGE(TAG, "authToken is null");
    return false;
  }

  if (cJSON_IsString(authTokenJson) == 0) {
    ESP_LOGE(TAG, "authToken is not a string");
    return false;
  }

  authToken = authTokenJson->valuestring;

  return true;
}

cJSON* BackendConfig::ToJSON(bool withSensitiveData) const {
  cJSON* root = cJSON_CreateObject();

  cJSON_AddStringToObject(root, "domain", domain.c_str());
  if (withSensitiveData) {
    cJSON_AddStringToObject(root, "authToken", authToken.c_str());
  }

  return root;
}
