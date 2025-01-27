#pragma once

#include "HttpMethod.h"
#include "HttpVersion.h"

#include <map>
#include <string>

class HttpRequestBodyParser {
public:
  bool TryParse(std::string_view view);

private:
  bool TryParseRequestLine(std::string_view view);

  HttpMethod _method;
  HttpVersion _version;
  std::string _url;
  std::map<std::string, std::string> _queryParams;
}
