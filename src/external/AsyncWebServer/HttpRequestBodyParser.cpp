#include <freertos/FreeRTOS.h>

#include "external/AsyncWebServer/HttpRequestBodyParser.h"

#include "Convert.h"
#include "external/AsyncWebServer/HttpMethod.h"
#include "external/AsyncWebServer/HttpVersion.h"
#include "util/HexUtils.h"
#include "util/StringUtils.h"

#include <algorithm>
#include <cstdint>
#include <map>
#include <string>
#include <string_view>

static bool TryUrlDecode(std::string_view view, std::string& decoded_out)
{
  decoded_out.reserve(view.length() - std::count(view.begin(), view.end(), '%') * 2);  // Every %XX will turn into one byte at output

  for (size_t i = 0; i < view.length(); ++i) {
    char encodedChar = view[i];

    uint8_t decodedChar;
    if (encodedChar == '%') [[unlikely]] {
      if (!OpenShock::HexUtils::TryParseHexPair(view[i], view[i + 1], decodedChar)) {
        return false;
      }
      i += 2;
    } else if (encodedChar == '+') [[unlikely]] {
      decodedChar = ' ';
    } else {
      decodedChar = encodedChar;  // normal ascii char
    }

    decoded_out.push_back(decodedChar);
  }

  return true;
}

static bool ParseQueryParams(std::string_view view, std::map<std::string, std::string>& params_out)
{
  while (!view.empty()) {
    auto pair = OpenShock::StringSplitByFirst(view, '&');
    view      = pair.second;

    pair = OpenShock::StringSplitByFirst(pair.first, '=');

    std::string key, value;
    if (!TryUrlDecode(pair.first, key) || !TryUrlDecode(pair.second, value)) {
      return false;
    }

    params_out.emplace(std::move(key), std::move(value));
  }

  return true;
}

static HttpMethod ParseHttpMethod(std::string_view view)
{
  using namespace std::string_view_literals;

  if (view == "GET"sv) return HttpMethod::GET;
  if (view == "POST"sv) return HttpMethod::POST;
  if (view == "DELETE"sv) return HttpMethod::DELETE;
  if (view == "PUT"sv) return HttpMethod::PUT;
  if (view == "PATCH"sv) return HttpMethod::PATCH;
  if (view == "CONNECT"sv) return HttpMethod::CONNECT;
  if (view == "HEAD"sv) return HttpMethod::HEAD;
  if (view == "OPTIONS"sv) return HttpMethod::OPTIONS;
  if (view == "TRACE"sv) return HttpMethod::TRACE;

  return HttpMethod::CUSTOM;
}

static bool ParseHttpVersion(std::string_view view, HttpVersion& http_version_out)
{
  using namespace std::string_view_literals;
  if (!OpenShock::StringHasPrefix(view, "HTTP/"sv)) {
    return false;
  }

  auto pair = OpenShock::StringSplitByFirst(view.substr(5), '.');

  if (!OpenShock::Convert::ToUint8(pair.first, http_version_out.major)) {
    return false;
  }

  if (!OpenShock::Convert::ToUint8(pair.second, http_version_out.minor)) {
    return false;
  }

  return true;
}

size_t HttpRequestBodyParser::Consume(std::string_view view)
{
  using namespace std::string_view_literals;

  if (_state == HttpParseState::Error) return 0;

  size_t offset = 0;

  // Parse the requestline
  if (_state == HttpParseState::RequestLine) {
    // Find end of requestline
    size_t pos = view.find("\r\n"sv);
    if (pos == std::string_view::npos) return 0;  // Need more data

    // Parse requestline
    if (!TryParseRequestLine(view.substr(0, pos))) {
      _state = HttpParseState::Error;
      return 0;
    }

    // Set state for continuation
    _state = HttpParseState::Headers;
    offset += pos + 2;
  }

  // Parse the headers
  if (_state == HttpParseState::Headers) {
    do {
      // Find end of header
      size_t pos = view.find("\r\n"sv, offset);
      if (pos == std::string_view::npos) return offset;  // Need more data
      if (pos == 0) break;                               // End of headers

      // Parse header
      if (!TryParseHeader(view.substr(0, pos))) {
        _state = HttpParseState::Error;
        return 0;
      }

      offset += pos + 2;
    } while (true);

    // Set state for continuation
    _state = HttpParseState::Body;
  }

  if (_state == HttpParseState::Body) {
  }

  return true;
}

bool HttpRequestBodyParser::TryParseRequestLine(std::string_view view)
{
  using namespace std::string_view_literals;

  std::size_t start_pos = 0, end_pos = 0;

  // Get request method
  end_pos = view.find(' ', start_pos);
  if (end_pos == std::string_view::npos) {
    return false;  // Should respond: 400 Bad Request
  }
  std::string_view method_str = view.substr(start_pos, end_pos);

  // Get request URI
  start_pos = end_pos + 1;
  end_pos   = view.find(' ', start_pos);
  if (end_pos == std::string_view::npos) {
    return false;  // Should respond: 400 Bad Request
  }
  std::string_view request_uri = view.substr(start_pos, end_pos);

  // Get request HTTP version
  std::string_view http_version_str = view.substr(end_pos + 1);

  _method = ParseHttpMethod(method_str);

  // Parse request URI query
  size_t query_pos = request_uri.find('?');
  if (query_pos != std::string_view::npos) {
    if (!ParseQueryParams(request_uri.substr(query_pos + 1), _queryParams)) {
      return false;
    }
    request_uri.remove_suffix(request_uri.size() - query_pos);
  }

  // Parse request URI
  if (!TryUrlDecode(request_uri, _url)) {
    return false;  // Should respond: 400 Bad Request
  }

  // Parse request HTTP version
  if (!ParseHttpVersion(http_version_str, _version)) {
    return false;  // Should respond: 400 Bad Request
  }

  return true;
}

bool HttpRequestBodyParser::TryParseHeader(std::string_view view)
{
  size_t pos = view.find(':');
  if (pos == std::string_view::npos) return false;

  std::string_view name  = view.substr(0, pos);
  std::string_view value = view.substr(pos + 1);

  // TODO: Validate these two

  _headers.emplace(name, value);
}
