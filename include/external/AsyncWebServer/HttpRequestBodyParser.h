#pragma once

#include "HttpMethod.h"
#include "HttpVersion.h"

#include <map>
#include <string>

class HttpRequestBodyParser {
  enum class HttpParseState {
    Error = -1,
    RequestLine,
    Headers,
    Body,
    Done
  };

public:
  size_t Consume(std::string_view view);
  bool IsBadRequest() const { return _state == HttpParseState::Error; }
  bool IsFinished() const { return _state == HttpParseState::Done; }

  HttpMethod method() const { return _method; }
  HttpVersion version() const { return _version; }
  HttpVersion versionMajor() const { return _version.major; }
  HttpVersion versionMinor() const { return _version.minor; }
  std::string_view url() const { return _url; }
  const std::map<std::string, std::string>& queryParams() const { return _queryParams; }
  const std::map<std::string, std::string>& headers() const { return _headers; }

private:
  bool TryParseRequestLine(std::string_view view);
  bool TryParseHeader(std::string_view view);

  HttpParseState _state;

  HttpMethod _method;
  HttpVersion _version;
  std::string _url;
  std::map<std::string, std::string> _queryParams;
  std::map<std::string, std::string> _headers;
}
