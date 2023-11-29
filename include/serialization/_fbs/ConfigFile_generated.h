// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_CONFIGFILE_OPENSHOCK_SERIALIZATION_CONFIGURATION_H_
#define FLATBUFFERS_GENERATED_CONFIGFILE_OPENSHOCK_SERIALIZATION_CONFIGURATION_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 23 &&
              FLATBUFFERS_VERSION_MINOR == 5 &&
              FLATBUFFERS_VERSION_REVISION == 26,
             "Non-compatible flatbuffers version included");

namespace OpenShock {
namespace Serialization {
namespace Configuration {

struct BSSID;

struct RFConfig;
struct RFConfigBuilder;

struct WiFiCredentials;
struct WiFiCredentialsBuilder;

struct WiFiConfig;
struct WiFiConfigBuilder;

struct CaptivePortalConfig;
struct CaptivePortalConfigBuilder;

struct BackendConfig;
struct BackendConfigBuilder;

struct Config;
struct ConfigBuilder;

FLATBUFFERS_MANUALLY_ALIGNED_STRUCT(1) BSSID FLATBUFFERS_FINAL_CLASS {
 private:
  uint8_t array_[6];

 public:
  struct Traits;
  static FLATBUFFERS_CONSTEXPR_CPP11 const char *GetFullyQualifiedName() {
    return "OpenShock.Serialization.Configuration.BSSID";
  }
  BSSID()
      : array_() {
  }
  BSSID(::flatbuffers::span<const uint8_t, 6> _array) {
    ::flatbuffers::CastToArray(array_).CopyFromSpan(_array);
  }
  const ::flatbuffers::Array<uint8_t, 6> *array() const {
    return &::flatbuffers::CastToArray(array_);
  }
};
FLATBUFFERS_STRUCT_END(BSSID, 6);

struct BSSID::Traits {
  using type = BSSID;
};

struct RFConfig FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef RFConfigBuilder Builder;
  struct Traits;
  static FLATBUFFERS_CONSTEXPR_CPP11 const char *GetFullyQualifiedName() {
    return "OpenShock.Serialization.Configuration.RFConfig";
  }
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_TX_PIN = 4,
    VT_KEEPALIVE_ENABLED = 6
  };
  /// The GPIO pin connected to the RF modulator's data pin for transmitting (TX)
  uint8_t tx_pin() const {
    return GetField<uint8_t>(VT_TX_PIN, 0);
  }
  /// Whether to transmit keepalive messages to keep the devices from entering sleep mode
  bool keepalive_enabled() const {
    return GetField<uint8_t>(VT_KEEPALIVE_ENABLED, 0) != 0;
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint8_t>(verifier, VT_TX_PIN, 1) &&
           VerifyField<uint8_t>(verifier, VT_KEEPALIVE_ENABLED, 1) &&
           verifier.EndTable();
  }
};

struct RFConfigBuilder {
  typedef RFConfig Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_tx_pin(uint8_t tx_pin) {
    fbb_.AddElement<uint8_t>(RFConfig::VT_TX_PIN, tx_pin, 0);
  }
  void add_keepalive_enabled(bool keepalive_enabled) {
    fbb_.AddElement<uint8_t>(RFConfig::VT_KEEPALIVE_ENABLED, static_cast<uint8_t>(keepalive_enabled), 0);
  }
  explicit RFConfigBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<RFConfig> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<RFConfig>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<RFConfig> CreateRFConfig(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    uint8_t tx_pin = 0,
    bool keepalive_enabled = false) {
  RFConfigBuilder builder_(_fbb);
  builder_.add_keepalive_enabled(keepalive_enabled);
  builder_.add_tx_pin(tx_pin);
  return builder_.Finish();
}

struct RFConfig::Traits {
  using type = RFConfig;
  static auto constexpr Create = CreateRFConfig;
};

struct WiFiCredentials FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef WiFiCredentialsBuilder Builder;
  struct Traits;
  static FLATBUFFERS_CONSTEXPR_CPP11 const char *GetFullyQualifiedName() {
    return "OpenShock.Serialization.Configuration.WiFiCredentials";
  }
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_ID = 4,
    VT_SSID = 6,
    VT_BSSID = 8,
    VT_PASSWORD = 10
  };
  uint8_t id() const {
    return GetField<uint8_t>(VT_ID, 0);
  }
  const ::flatbuffers::String *ssid() const {
    return GetPointer<const ::flatbuffers::String *>(VT_SSID);
  }
  const OpenShock::Serialization::Configuration::BSSID *bssid() const {
    return GetStruct<const OpenShock::Serialization::Configuration::BSSID *>(VT_BSSID);
  }
  const ::flatbuffers::String *password() const {
    return GetPointer<const ::flatbuffers::String *>(VT_PASSWORD);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint8_t>(verifier, VT_ID, 1) &&
           VerifyOffset(verifier, VT_SSID) &&
           verifier.VerifyString(ssid()) &&
           VerifyField<OpenShock::Serialization::Configuration::BSSID>(verifier, VT_BSSID, 1) &&
           VerifyOffset(verifier, VT_PASSWORD) &&
           verifier.VerifyString(password()) &&
           verifier.EndTable();
  }
};

struct WiFiCredentialsBuilder {
  typedef WiFiCredentials Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_id(uint8_t id) {
    fbb_.AddElement<uint8_t>(WiFiCredentials::VT_ID, id, 0);
  }
  void add_ssid(::flatbuffers::Offset<::flatbuffers::String> ssid) {
    fbb_.AddOffset(WiFiCredentials::VT_SSID, ssid);
  }
  void add_bssid(const OpenShock::Serialization::Configuration::BSSID *bssid) {
    fbb_.AddStruct(WiFiCredentials::VT_BSSID, bssid);
  }
  void add_password(::flatbuffers::Offset<::flatbuffers::String> password) {
    fbb_.AddOffset(WiFiCredentials::VT_PASSWORD, password);
  }
  explicit WiFiCredentialsBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<WiFiCredentials> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<WiFiCredentials>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<WiFiCredentials> CreateWiFiCredentials(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    uint8_t id = 0,
    ::flatbuffers::Offset<::flatbuffers::String> ssid = 0,
    const OpenShock::Serialization::Configuration::BSSID *bssid = nullptr,
    ::flatbuffers::Offset<::flatbuffers::String> password = 0) {
  WiFiCredentialsBuilder builder_(_fbb);
  builder_.add_password(password);
  builder_.add_bssid(bssid);
  builder_.add_ssid(ssid);
  builder_.add_id(id);
  return builder_.Finish();
}

struct WiFiCredentials::Traits {
  using type = WiFiCredentials;
  static auto constexpr Create = CreateWiFiCredentials;
};

inline ::flatbuffers::Offset<WiFiCredentials> CreateWiFiCredentialsDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    uint8_t id = 0,
    const char *ssid = nullptr,
    const OpenShock::Serialization::Configuration::BSSID *bssid = nullptr,
    const char *password = nullptr) {
  auto ssid__ = ssid ? _fbb.CreateString(ssid) : 0;
  auto password__ = password ? _fbb.CreateString(password) : 0;
  return OpenShock::Serialization::Configuration::CreateWiFiCredentials(
      _fbb,
      id,
      ssid__,
      bssid,
      password__);
}

struct WiFiConfig FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef WiFiConfigBuilder Builder;
  struct Traits;
  static FLATBUFFERS_CONSTEXPR_CPP11 const char *GetFullyQualifiedName() {
    return "OpenShock.Serialization.Configuration.WiFiConfig";
  }
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_AP_SSID = 4,
    VT_HOSTNAME = 6,
    VT_CREDENTIALS = 8
  };
  const ::flatbuffers::String *ap_ssid() const {
    return GetPointer<const ::flatbuffers::String *>(VT_AP_SSID);
  }
  const ::flatbuffers::String *hostname() const {
    return GetPointer<const ::flatbuffers::String *>(VT_HOSTNAME);
  }
  const ::flatbuffers::Vector<::flatbuffers::Offset<OpenShock::Serialization::Configuration::WiFiCredentials>> *credentials() const {
    return GetPointer<const ::flatbuffers::Vector<::flatbuffers::Offset<OpenShock::Serialization::Configuration::WiFiCredentials>> *>(VT_CREDENTIALS);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_AP_SSID) &&
           verifier.VerifyString(ap_ssid()) &&
           VerifyOffset(verifier, VT_HOSTNAME) &&
           verifier.VerifyString(hostname()) &&
           VerifyOffset(verifier, VT_CREDENTIALS) &&
           verifier.VerifyVector(credentials()) &&
           verifier.VerifyVectorOfTables(credentials()) &&
           verifier.EndTable();
  }
};

struct WiFiConfigBuilder {
  typedef WiFiConfig Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_ap_ssid(::flatbuffers::Offset<::flatbuffers::String> ap_ssid) {
    fbb_.AddOffset(WiFiConfig::VT_AP_SSID, ap_ssid);
  }
  void add_hostname(::flatbuffers::Offset<::flatbuffers::String> hostname) {
    fbb_.AddOffset(WiFiConfig::VT_HOSTNAME, hostname);
  }
  void add_credentials(::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<OpenShock::Serialization::Configuration::WiFiCredentials>>> credentials) {
    fbb_.AddOffset(WiFiConfig::VT_CREDENTIALS, credentials);
  }
  explicit WiFiConfigBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<WiFiConfig> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<WiFiConfig>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<WiFiConfig> CreateWiFiConfig(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<::flatbuffers::String> ap_ssid = 0,
    ::flatbuffers::Offset<::flatbuffers::String> hostname = 0,
    ::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<OpenShock::Serialization::Configuration::WiFiCredentials>>> credentials = 0) {
  WiFiConfigBuilder builder_(_fbb);
  builder_.add_credentials(credentials);
  builder_.add_hostname(hostname);
  builder_.add_ap_ssid(ap_ssid);
  return builder_.Finish();
}

struct WiFiConfig::Traits {
  using type = WiFiConfig;
  static auto constexpr Create = CreateWiFiConfig;
};

inline ::flatbuffers::Offset<WiFiConfig> CreateWiFiConfigDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    const char *ap_ssid = nullptr,
    const char *hostname = nullptr,
    const std::vector<::flatbuffers::Offset<OpenShock::Serialization::Configuration::WiFiCredentials>> *credentials = nullptr) {
  auto ap_ssid__ = ap_ssid ? _fbb.CreateString(ap_ssid) : 0;
  auto hostname__ = hostname ? _fbb.CreateString(hostname) : 0;
  auto credentials__ = credentials ? _fbb.CreateVector<::flatbuffers::Offset<OpenShock::Serialization::Configuration::WiFiCredentials>>(*credentials) : 0;
  return OpenShock::Serialization::Configuration::CreateWiFiConfig(
      _fbb,
      ap_ssid__,
      hostname__,
      credentials__);
}

struct CaptivePortalConfig FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef CaptivePortalConfigBuilder Builder;
  struct Traits;
  static FLATBUFFERS_CONSTEXPR_CPP11 const char *GetFullyQualifiedName() {
    return "OpenShock.Serialization.Configuration.CaptivePortalConfig";
  }
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_ALWAYS_ENABLED = 4
  };
  /// Whether the captive portal is forced to be enabled
  /// The captive portal will otherwise shut down when a gateway connection is established
  bool always_enabled() const {
    return GetField<uint8_t>(VT_ALWAYS_ENABLED, 0) != 0;
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint8_t>(verifier, VT_ALWAYS_ENABLED, 1) &&
           verifier.EndTable();
  }
};

struct CaptivePortalConfigBuilder {
  typedef CaptivePortalConfig Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_always_enabled(bool always_enabled) {
    fbb_.AddElement<uint8_t>(CaptivePortalConfig::VT_ALWAYS_ENABLED, static_cast<uint8_t>(always_enabled), 0);
  }
  explicit CaptivePortalConfigBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<CaptivePortalConfig> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<CaptivePortalConfig>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<CaptivePortalConfig> CreateCaptivePortalConfig(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    bool always_enabled = false) {
  CaptivePortalConfigBuilder builder_(_fbb);
  builder_.add_always_enabled(always_enabled);
  return builder_.Finish();
}

struct CaptivePortalConfig::Traits {
  using type = CaptivePortalConfig;
  static auto constexpr Create = CreateCaptivePortalConfig;
};

struct BackendConfig FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef BackendConfigBuilder Builder;
  struct Traits;
  static FLATBUFFERS_CONSTEXPR_CPP11 const char *GetFullyQualifiedName() {
    return "OpenShock.Serialization.Configuration.BackendConfig";
  }
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_DOMAIN = 4,
    VT_AUTH_TOKEN = 6
  };
  /// Domain name of the backend server, e.g. "api.shocklink.net"
  const ::flatbuffers::String *domain() const {
    return GetPointer<const ::flatbuffers::String *>(VT_DOMAIN);
  }
  /// Authentication token for the backend server
  const ::flatbuffers::String *auth_token() const {
    return GetPointer<const ::flatbuffers::String *>(VT_AUTH_TOKEN);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_DOMAIN) &&
           verifier.VerifyString(domain()) &&
           VerifyOffset(verifier, VT_AUTH_TOKEN) &&
           verifier.VerifyString(auth_token()) &&
           verifier.EndTable();
  }
};

struct BackendConfigBuilder {
  typedef BackendConfig Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_domain(::flatbuffers::Offset<::flatbuffers::String> domain) {
    fbb_.AddOffset(BackendConfig::VT_DOMAIN, domain);
  }
  void add_auth_token(::flatbuffers::Offset<::flatbuffers::String> auth_token) {
    fbb_.AddOffset(BackendConfig::VT_AUTH_TOKEN, auth_token);
  }
  explicit BackendConfigBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<BackendConfig> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<BackendConfig>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<BackendConfig> CreateBackendConfig(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<::flatbuffers::String> domain = 0,
    ::flatbuffers::Offset<::flatbuffers::String> auth_token = 0) {
  BackendConfigBuilder builder_(_fbb);
  builder_.add_auth_token(auth_token);
  builder_.add_domain(domain);
  return builder_.Finish();
}

struct BackendConfig::Traits {
  using type = BackendConfig;
  static auto constexpr Create = CreateBackendConfig;
};

inline ::flatbuffers::Offset<BackendConfig> CreateBackendConfigDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    const char *domain = nullptr,
    const char *auth_token = nullptr) {
  auto domain__ = domain ? _fbb.CreateString(domain) : 0;
  auto auth_token__ = auth_token ? _fbb.CreateString(auth_token) : 0;
  return OpenShock::Serialization::Configuration::CreateBackendConfig(
      _fbb,
      domain__,
      auth_token__);
}

struct Config FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef ConfigBuilder Builder;
  struct Traits;
  static FLATBUFFERS_CONSTEXPR_CPP11 const char *GetFullyQualifiedName() {
    return "OpenShock.Serialization.Configuration.Config";
  }
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_RF = 4,
    VT_WIFI = 6,
    VT_CAPTIVE_PORTAL = 8,
    VT_BACKEND = 10
  };
  const OpenShock::Serialization::Configuration::RFConfig *rf() const {
    return GetPointer<const OpenShock::Serialization::Configuration::RFConfig *>(VT_RF);
  }
  const OpenShock::Serialization::Configuration::WiFiConfig *wifi() const {
    return GetPointer<const OpenShock::Serialization::Configuration::WiFiConfig *>(VT_WIFI);
  }
  const OpenShock::Serialization::Configuration::CaptivePortalConfig *captive_portal() const {
    return GetPointer<const OpenShock::Serialization::Configuration::CaptivePortalConfig *>(VT_CAPTIVE_PORTAL);
  }
  const OpenShock::Serialization::Configuration::BackendConfig *backend() const {
    return GetPointer<const OpenShock::Serialization::Configuration::BackendConfig *>(VT_BACKEND);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_RF) &&
           verifier.VerifyTable(rf()) &&
           VerifyOffset(verifier, VT_WIFI) &&
           verifier.VerifyTable(wifi()) &&
           VerifyOffset(verifier, VT_CAPTIVE_PORTAL) &&
           verifier.VerifyTable(captive_portal()) &&
           VerifyOffset(verifier, VT_BACKEND) &&
           verifier.VerifyTable(backend()) &&
           verifier.EndTable();
  }
};

struct ConfigBuilder {
  typedef Config Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_rf(::flatbuffers::Offset<OpenShock::Serialization::Configuration::RFConfig> rf) {
    fbb_.AddOffset(Config::VT_RF, rf);
  }
  void add_wifi(::flatbuffers::Offset<OpenShock::Serialization::Configuration::WiFiConfig> wifi) {
    fbb_.AddOffset(Config::VT_WIFI, wifi);
  }
  void add_captive_portal(::flatbuffers::Offset<OpenShock::Serialization::Configuration::CaptivePortalConfig> captive_portal) {
    fbb_.AddOffset(Config::VT_CAPTIVE_PORTAL, captive_portal);
  }
  void add_backend(::flatbuffers::Offset<OpenShock::Serialization::Configuration::BackendConfig> backend) {
    fbb_.AddOffset(Config::VT_BACKEND, backend);
  }
  explicit ConfigBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<Config> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<Config>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<Config> CreateConfig(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<OpenShock::Serialization::Configuration::RFConfig> rf = 0,
    ::flatbuffers::Offset<OpenShock::Serialization::Configuration::WiFiConfig> wifi = 0,
    ::flatbuffers::Offset<OpenShock::Serialization::Configuration::CaptivePortalConfig> captive_portal = 0,
    ::flatbuffers::Offset<OpenShock::Serialization::Configuration::BackendConfig> backend = 0) {
  ConfigBuilder builder_(_fbb);
  builder_.add_backend(backend);
  builder_.add_captive_portal(captive_portal);
  builder_.add_wifi(wifi);
  builder_.add_rf(rf);
  return builder_.Finish();
}

struct Config::Traits {
  using type = Config;
  static auto constexpr Create = CreateConfig;
};

}  // namespace Configuration
}  // namespace Serialization
}  // namespace OpenShock

#endif  // FLATBUFFERS_GENERATED_CONFIGFILE_OPENSHOCK_SERIALIZATION_CONFIGURATION_H_
