/*
  Asynchronous WebServer library for Espressif MCUs

  Copyright (c) 2016 Hristo Gochkov. All rights reserved.
  This file is part of the esp8266 core for Arduino environment.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include "external/AsyncWebServer/ESPAsyncWebServer.h"
#include "external/AsyncWebServer/WebResponseImpl.h"

#include "Convert.h"
#include "util/HexUtils.h"

static const std::string SharedEmptyString = std::string();

#define __is_param_char(c) ((c) && ((c) != '{') && ((c) != '[') && ((c) != '&') && ((c) != '='))

static bool httpTryBasicUriDecode(std::string_view uri, std::string& uri_out)
{
  uri_out.clear();

  if (uri.empty()) return false;

  uri_out.reserve(uri.length());

  for (std::size_t i = 0; i < uri.length(); ++i) {
    char c = uri[i];

    // Escaped character handling
    if (c == '%') [[unlikely]] {
      // Check if theres enough space for the two hex chars
      if (i + 2 >= uri.length()) {
        uri_out.clear();
        return false;
      }

      // Decode the hex characters
      uint8_t decoded = 0;
      if (!OpenShock::HexUtils::TryParseHexPair(uri[i + 1], uri[i + 2], decoded)) {
        uri_out.clear();
        return false;
      }

      // Push back the decoded character
      uri_out.push_back(static_cast<char>(decoded));

      // Skip the hex characters
      i += 2;

      continue;
    }

    // Fail on whitespace
    if (c == ' ' || c == '\t' || c == '\r' || c == '\n') [[unlikely]] {
      uri_out.clear();
      return false;
    }

    // Push back the hopefully valid character
    uri_out.push_back(c);
  }

  return true;
}

static bool httpParseMethod(std::string_view str, HttpRequestMethod& method_out)
{
  using namespace std::string_view_literals;

  if (str == "GET"sv) {
    method_out = HTTP_GET;
  } else if (str == "POST"sv) {
    method_out = HTTP_POST;
  } else if (str == "DELETE"sv) {
    method_out = HTTP_DELETE;
  } else if (str == "PUT"sv) {
    method_out = HTTP_PUT;
  } else if (str == "PATCH"sv) {
    method_out = HTTP_PATCH;
  } else if (str == "HEAD"sv) {
    method_out = HTTP_HEAD;
  } else if (str == "OPTIONS"sv) {
    method_out = HTTP_OPTIONS;
  } else if (str == "CONNECT"sv) {
    method_out = HTTP_CONNECT;
  } else if (str == "TRACE"sv) {
    method_out = HTTP_TRACE;
  } else {
    return false;
  }

  return true;
}
static bool httpParseHttpVersion(std::string_view str, HttpVersion& http_version_out)
{
  using namespace std::string_view_literals;
  if (!OpenShock::StringHasPrefix(str, "HTTP/"sv)) {
    return false;
  }

  auto pair = OpenShock::StringSplitByFirst(str.substr(5), '.');

  if (!OpenShock::Convert::ToUint8(pair.first, http_version_out.major)) {
    return false;
  }

  if (!OpenShock::Convert::ToUint8(pair.second, http_version_out.minor)) {
    return false;
  }

  return true;
}

AsyncWebServerRequest::AsyncWebServerRequest(AsyncWebServer* s, AsyncClient* c)
  : _client(c)
  , _server(s)
  , _handler(NULL)
  , _response(NULL)
  , _temp()
  , _parseState(0)
  , _version {}
  , _method(HTTP_ANY)
  , _url()
  , _host()
  , _contentType()
  , _boundary()
  , _authorization()
  , _reqconntype(RCT_HTTP)
  , _isDigest(false)
  , _isMultipart(false)
  , _isPlainPost(false)
  , _expectingContinue(false)
  , _contentLength(0)
  , _parsedLength(0)
  , _headers(LinkedList<AsyncWebHeader*>([](AsyncWebHeader* h) { delete h; }))
  , _params(LinkedList<AsyncWebParameter*>([](AsyncWebParameter* p) { delete p; }))
  , _pathParams(LinkedList<std::string*>([](std::string* p) { delete p; }))
  , _multiParseState(0)
  , _boundaryPosition(0)
  , _itemStartIndex(0)
  , _itemSize(0)
  , _itemName()
  , _itemFilename()
  , _itemType()
  , _itemValue()
  , _itemBuffer(0)
  , _itemBufferIndex(0)
  , _itemIsFile(false)
  , _tempObject(NULL)
{
  c->onError([](void* r, AsyncClient* c, int8_t error) {
    (void)c;
    AsyncWebServerRequest* req = (AsyncWebServerRequest*)r;
    req->_onError(error);
  }, this);
  c->onAck([](void* r, AsyncClient* c, size_t len, uint32_t time) {
    (void)c;
    AsyncWebServerRequest* req = (AsyncWebServerRequest*)r;
    req->_onAck(len, time);
  }, this);
  c->onDisconnect([](void* r, AsyncClient* c) {
    AsyncWebServerRequest* req = (AsyncWebServerRequest*)r;
    req->_onDisconnect();
    delete c;
  }, this);
  c->onTimeout([](void* r, AsyncClient* c, uint32_t time) {
    (void)c;
    AsyncWebServerRequest* req = (AsyncWebServerRequest*)r;
    req->_onTimeout(time);
  }, this);
  c->onData([](void* r, AsyncClient* c, void* buf, size_t len) {
    (void)c;
    AsyncWebServerRequest* req = (AsyncWebServerRequest*)r;
    req->_onData(buf, len);
  }, this);
  c->onPoll([](void* r, AsyncClient* c) {
    (void)c;
    AsyncWebServerRequest* req = (AsyncWebServerRequest*)r;
    req->_onPoll();
  }, this);
}

AsyncWebServerRequest::~AsyncWebServerRequest()
{
  _headers.free();

  _params.free();
  _pathParams.free();

  if (_response != NULL) {
    delete _response;
  }

  if (_tempObject != NULL) {
    free(_tempObject);
  }

  if (_tempFile) {
    _tempFile.close();
  }
}

void AsyncWebServerRequest::_onData(void* buf, size_t len)
{
  size_t i = 0;
  while (true) {
    if (_parseState < PARSE_REQ_BODY) {
      // Find new line in buf
      char* str = (char*)buf;
      for (i = 0; i < len; i++) {
        if (str[i] == '\n') {
          break;
        }
      }
      if (i == len) {  // No new line, just add the buffer in _temp
        char ch      = str[len - 1];
        str[len - 1] = 0;
        _temp.reserve(_temp.length() + len);
        _temp += str;
        _temp += ch;
      } else {                                 // Found new line - extract it and parse
        str[i] = 0;                            // Terminate the string at the end of the line.
        _temp += str;
        _temp = OpenShock::StringTrim(_temp);  // TODO: This can be optimized if only right side needs trim
        _parseLine();
        if (++i < len) {
          // Still have more buffer to process
          buf = str + i;
          len -= i;
          continue;
        }
      }
    } else if (_parseState == PARSE_REQ_BODY) {
      // A handler should be already attached at this point in _parseLine function.
      // If handler does nothing (_onRequest is NULL), we don't need to really parse the body.
      const bool needParse = _handler && !_handler->isRequestHandlerTrivial();
      if (_isMultipart) {
        if (needParse) {
          size_t i;
          for (i = 0; i < len; i++) {
            _parseMultipartPostByte(((uint8_t*)buf)[i], i == len - 1);
            _parsedLength++;
          }
        } else
          _parsedLength += len;
      } else {
        if (_parsedLength == 0) {
          using namespace std::string_view_literals;
          if (OpenShock::StringHasPrefix(_contentType, "application/x-www-form-urlencoded"sv)) {
            _isPlainPost = true;
          } else if (_contentType == "text/plain" && __is_param_char(((char*)buf)[0])) {
            size_t i = 0;
            while (i < len && __is_param_char(((char*)buf)[i++]));
            if (i < len && ((char*)buf)[i - 1] == '=') {
              _isPlainPost = true;
            }
          }
        }
        if (!_isPlainPost) {
          // check if authenticated before calling the body
          if (_handler) _handler->handleBody(this, (uint8_t*)buf, len, _parsedLength, _contentLength);
          _parsedLength += len;
        } else if (needParse) {
          size_t i;
          for (i = 0; i < len; i++) {
            _parsedLength++;
            _parsePlainPostChar(((uint8_t*)buf)[i]);
          }
        } else {
          _parsedLength += len;
        }
      }
      if (_parsedLength == _contentLength) {
        _parseState = PARSE_REQ_END;
        // check if authenticated before calling handleRequest and request auth instead
        if (_handler)
          _handler->handleRequest(this);
        else
          send(501);
      }
    }
    break;
  }
}

void AsyncWebServerRequest::_onPoll()
{
  // os_printf("p\n");
  if (_response != NULL && _client != NULL && _client->canSend()) {
    if (!_response->_finished()) {
      _response->_ack(this, 0, 0);
    } else {
      AsyncWebServerResponse* r = _response;
      _response                 = NULL;
      delete r;

      _client->close();
    }
  }
}

void AsyncWebServerRequest::_onAck(size_t len, uint32_t time)
{
  // os_printf("a:%u:%u\n", len, time);
  if (_response != NULL) {
    if (!_response->_finished()) {
      _response->_ack(this, len, time);
    } else {
      AsyncWebServerResponse* r = _response;
      _response                 = NULL;
      delete r;
    }
  }
}

void AsyncWebServerRequest::_onError(int8_t error)
{
  (void)error;
}

void AsyncWebServerRequest::_onTimeout(uint32_t time)
{
  (void)time;
  // os_printf("TIMEOUT: %u, state: %s\n", time, _client->stateToString());
  _client->close();
}

void AsyncWebServerRequest::onDisconnect(ArDisconnectHandler fn)
{
  _onDisconnectfn = fn;
}

void AsyncWebServerRequest::_onDisconnect()
{
  // os_printf("d\n");
  if (_onDisconnectfn) {
    _onDisconnectfn();
  }
  _server->_handleDisconnect(this);
}

void AsyncWebServerRequest::_addParam(AsyncWebParameter* p)
{
  _params.add(p);
}

void AsyncWebServerRequest::_addPathParam(const char* p)
{
  _pathParams.add(new std::string(p));
}

void AsyncWebServerRequest::_parseQueryParams(std::string_view params)
{
  while (!params.empty()) {
    auto pair = OpenShock::StringSplitByFirst(params, '&');
    params    = pair.second;

    pair = OpenShock::StringSplitByFirst(pair.first, '=');

    _addParam(new AsyncWebParameter(urlDecode(pair.first), urlDecode(pair.second)));
  }
}

bool AsyncWebServerRequest::_parseReqHead()
{
  using namespace std::string_view_literals;

  std::string_view body = _temp;

  std::size_t start_pos = 0, end_pos = 0;

  // Get request method
  end_pos = body.find(' ', start_pos);
  if (end_pos == std::string_view::npos) {
    return false;  // Should respond: 400 Bad Request
  }
  std::string_view method_str = body.substr(start_pos, end_pos);

  // Get request URI
  start_pos = end_pos + 1;
  end_pos   = body.find(' ', start_pos);
  if (end_pos == std::string_view::npos) {
    return false;  // Should respond: 400 Bad Request
  }
  std::string_view request_uri = body.substr(start_pos, end_pos);

  // Get request HTTP version
  start_pos = end_pos + 1;
  end_pos   = body.find("\r\n"sv, start_pos);
  if (end_pos == std::string_view::npos) {
    return false;  // Should respond: 400 Bad Request
  }
  std::string_view http_version_str = body.substr(start_pos, end_pos);

  // Parse request method
  if (!httpParseMethod(method_str, _method)) {
    return false;  // Should respond: 405 Method Not Allowed
  }

  // Parse request URI query
  size_t query_pos = request_uri.find('?');
  if (query_pos != std::string_view::npos) {
    _parseQueryParams(request_uri.substr(query_pos + 1));
    request_uri.remove_suffix(request_uri.size() - query_pos);
  }

  // Parse request URI
  if (!httpTryBasicUriDecode(request_uri, _url)) {
    return false;  // Should respond: 400 Bad Request
  }

  // Parse request HTTP version
  if (!httpParseHttpVersion(http_version_str, _version)) {
    return false;  // Should respond: 400 Bad Request
  }

  return true;
}

bool AsyncWebServerRequest::_parseReqHeader(std::string_view header)
{
  auto pair = OpenShock::StringSplitByFirst(header, ':');

  using namespace std::string_view_literals;

  if (OpenShock::StringIEquals(pair.first, "Host"sv)) {
    _host = pair.second;
  } else if (OpenShock::StringIEquals(pair.first, "Content-Type"sv)) {
    _contentType = OpenShock::StringBeforeFirst(pair.second, ';');
    if (OpenShock::StringHasPrefix(_contentType, "multipart/"sv)) {
      _boundary = OpenShock::StringAfterFirst(pair.second, '=');
      _boundary.replace("\"", "");  // TODO: Optimize this by merging with string_view assignment
      _isMultipart = true;
    }
  } else if (OpenShock::StringIEquals(pair.first, "Content-Length"sv)) {
    if (!OpenShock::Convert::ToSizeT(pair.second, _contentLength)) {
      return false;
    }
  } else if (OpenShock::StringIEquals(pair.first, "Expect"sv)) {
    if (pair.second == "100-continue") {
      _expectingContinue = true;
    }
  } else if (OpenShock::StringIEquals(pair.first, "Authorization"sv)) {
    if (OpenShock::StringHasPrefixIC(pair.second, "Basic"sv)) {
      _authorization = pair.second.substr(6);  // TODO: validate seperator
    } else if (OpenShock::StringHasPrefixIC(pair.second, "Digest"sv)) {
      _isDigest      = true;
      _authorization = pair.second.substr(7);  // TODO: validate seperator
    }
  } else if (OpenShock::StringIEquals(pair.first, "Upgrade"sv)) {
    if (OpenShock::StringIEquals(pair.second, "websocket"sv)) {
      // WebSocket request can be uniquely identified by header: [Upgrade: websocket]
      _reqconntype = RCT_WS;
    }
  } else if (OpenShock::StringIEquals(pair.first, "Accept"sv)) {
    if (OpenShock::StringIContains(pair.second, "text/event-stream"sv)) {
      // WebEvent request can be uniquely identified by header:  [Accept: text/event-stream]
      _reqconntype = RCT_EVENT;
    }
  }

  _headers.add(new AsyncWebHeader(pair.first, pair.second));

  _temp.clear();
  return true;
}

void AsyncWebServerRequest::_parsePlainPostChar(uint8_t data)
{
  if (data && (char)data != '&') {
    _temp += (char)data;
    return;
  }

  if (_parsedLength != _contentLength) return;

  using namespace std::string_view_literals;

  std::string_view name  = "body"sv;
  std::string_view value = _temp;

  if (!OpenShock::StringHasPrefix(value, '{') && !OpenShock::StringHasPrefix(value, '[')) {
    size_t equals_pos = value.find('=');
    if (equals_pos != std::string_view::npos) {
      name  = value.substr(0, equals_pos);
      value = value.substr(equals_pos + 1);
    }
  }

  _addParam(new AsyncWebParameter(urlDecode(name), urlDecode(value), true));
  _temp = std::string();
}

void AsyncWebServerRequest::_handleUploadByte(uint8_t data, bool last)
{
  _itemBuffer[_itemBufferIndex++] = data;

  if (last || _itemBufferIndex == 1460) {
    // check if authenticated before calling the upload
    if (_handler) _handler->handleUpload(this, _itemFilename, _itemSize - _itemBufferIndex, _itemBuffer, _itemBufferIndex, false);
    _itemBufferIndex = 0;
  }
}

enum {
  EXPECT_BOUNDARY,
  PARSE_HEADERS,
  WAIT_FOR_RETURN1,
  EXPECT_FEED1,
  EXPECT_DASH1,
  EXPECT_DASH2,
  BOUNDARY_OR_DATA,
  DASH3_OR_RETURN2,
  EXPECT_FEED2,
  PARSING_FINISHED,
  PARSE_ERROR
};

void AsyncWebServerRequest::_parseMultipartPostByte(uint8_t data, bool last)
{
#define itemWriteByte(b)          \
  do {                            \
    _itemSize++;                  \
    if (_itemIsFile)              \
      _handleUploadByte(b, last); \
    else                          \
      _itemValue += (char)(b);    \
  } while (0)

  if (!_parsedLength) {
    _multiParseState = EXPECT_BOUNDARY;
    _temp            = std::string();
    _itemName        = std::string();
    _itemFilename    = std::string();
    _itemType        = std::string();
  }

  if (_multiParseState == WAIT_FOR_RETURN1) {
    if (data != '\r') {
      itemWriteByte(data);
    } else {
      _multiParseState = EXPECT_FEED1;
    }
  } else if (_multiParseState == EXPECT_BOUNDARY) {
    if (_parsedLength < 2 && data != '-') {
      _multiParseState = PARSE_ERROR;
      return;
    } else if (_parsedLength - 2 < _boundary.length() && _boundary.c_str()[_parsedLength - 2] != data) {
      _multiParseState = PARSE_ERROR;
      return;
    } else if (_parsedLength - 2 == _boundary.length() && data != '\r') {
      _multiParseState = PARSE_ERROR;
      return;
    } else if (_parsedLength - 3 == _boundary.length()) {
      if (data != '\n') {
        _multiParseState = PARSE_ERROR;
        return;
      }
      _multiParseState = PARSE_HEADERS;
      _itemIsFile      = false;
    }
  } else if (_multiParseState == PARSE_HEADERS) {
    if ((char)data != '\r' && (char)data != '\n') _temp += (char)data;
    if ((char)data == '\n') {
      using namespace std::string_view_literals;
      std::string_view view = _temp;
      if (!view.empty()) {
        if (OpenShock::StringTryRemovePrefixIC(view, "Content-Type:"sv)) {
          _itemType   = OpenShock::StringTrim(view);
          _itemIsFile = true;
        } else if (OpenShock::StringTryRemovePrefixIC(view, "Content-Disposition:"sv)) {
          for (const auto part : OpenShock::StringSplit(view, ';')) {
            auto trimmed = OpenShock::StringTrim(part);

            size_t equalsPos = trimmed.find('=');
            if (equalsPos == std::string_view::npos) {
              if (trimmed != "form-data"sv) {
                _multiParseState = PARSE_ERROR;
                return;
              }
            } else {
              std::string_view key   = view.substr(0, equalsPos);
              std::string_view value = view.substr(equalsPos + 1);
            }
          }
          _temp = _temp.substring(_temp.indexOf(';') + 2);
          while (_temp.indexOf(';') > 0) {
            std::string name    = _temp.substring(0, _temp.indexOf('='));
            std::string nameVal = _temp.substring(_temp.indexOf('=') + 2, _temp.indexOf(';') - 1);
            if (name == "name") {
              _itemName = nameVal;
            } else if (name == "filename") {
              _itemFilename = nameVal;
              _itemIsFile   = true;
            }
            _temp = _temp.substring(_temp.indexOf(';') + 2);
          }
          std::string name    = _temp.substring(0, _temp.indexOf('='));
          std::string nameVal = _temp.substring(_temp.indexOf('=') + 2, _temp.length() - 1);
          if (name == "name") {
            _itemName = nameVal;
          } else if (name == "filename") {
            _itemFilename = nameVal;
            _itemIsFile   = true;
          }
        }
        _temp = std::string();
      } else {
        _multiParseState = WAIT_FOR_RETURN1;
        // value starts from here
        _itemSize       = 0;
        _itemStartIndex = _parsedLength;
        _itemValue      = std::string();
        if (_itemIsFile) {
          if (_itemBuffer) free(_itemBuffer);
          _itemBuffer = (uint8_t*)malloc(1460);
          if (_itemBuffer == NULL) {
            _multiParseState = PARSE_ERROR;
            return;
          }
          _itemBufferIndex = 0;
        }
      }
    }
  } else if (_multiParseState == EXPECT_FEED1) {
    if (data != '\n') {
      _multiParseState = WAIT_FOR_RETURN1;
      itemWriteByte('\r');
      _parseMultipartPostByte(data, last);
    } else {
      _multiParseState = EXPECT_DASH1;
    }
  } else if (_multiParseState == EXPECT_DASH1) {
    if (data != '-') {
      _multiParseState = WAIT_FOR_RETURN1;
      itemWriteByte('\r');
      itemWriteByte('\n');
      _parseMultipartPostByte(data, last);
    } else {
      _multiParseState = EXPECT_DASH2;
    }
  } else if (_multiParseState == EXPECT_DASH2) {
    if (data != '-') {
      _multiParseState = WAIT_FOR_RETURN1;
      itemWriteByte('\r');
      itemWriteByte('\n');
      itemWriteByte('-');
      _parseMultipartPostByte(data, last);
    } else {
      _multiParseState  = BOUNDARY_OR_DATA;
      _boundaryPosition = 0;
    }
  } else if (_multiParseState == BOUNDARY_OR_DATA) {
    if (_boundaryPosition < _boundary.length() && _boundary.c_str()[_boundaryPosition] != data) {
      _multiParseState = WAIT_FOR_RETURN1;
      itemWriteByte('\r');
      itemWriteByte('\n');
      itemWriteByte('-');
      itemWriteByte('-');
      uint8_t i;
      for (i = 0; i < _boundaryPosition; i++) itemWriteByte(_boundary.c_str()[i]);
      _parseMultipartPostByte(data, last);
    } else if (_boundaryPosition == _boundary.length() - 1) {
      _multiParseState = DASH3_OR_RETURN2;
      if (!_itemIsFile) {
        _addParam(new AsyncWebParameter(_itemName, _itemValue, true));
      } else {
        if (_itemSize) {
          // check if authenticated before calling the upload
          if (_handler) _handler->handleUpload(this, _itemFilename, _itemSize - _itemBufferIndex, _itemBuffer, _itemBufferIndex, true);
          _itemBufferIndex = 0;
          _addParam(new AsyncWebParameter(_itemName, _itemFilename, true, true, _itemSize));
        }
        free(_itemBuffer);
        _itemBuffer = NULL;
      }
    } else {
      _boundaryPosition++;
    }
  } else if (_multiParseState == DASH3_OR_RETURN2) {
    if (data == '-' && (_contentLength - _parsedLength - 4) != 0) {
      // os_printf("ERROR: The parser got to the end of the POST but is expecting %u bytes more!\nDrop an issue so we can have more info on the matter!\n", _contentLength - _parsedLength - 4);
      _contentLength = _parsedLength + 4;  // lets close the request gracefully
    }
    if (data == '\r') {
      _multiParseState = EXPECT_FEED2;
    } else if (data == '-' && _contentLength == (_parsedLength + 4)) {
      _multiParseState = PARSING_FINISHED;
    } else {
      _multiParseState = WAIT_FOR_RETURN1;
      itemWriteByte('\r');
      itemWriteByte('\n');
      itemWriteByte('-');
      itemWriteByte('-');
      uint32_t i;
      for (i = 0; i < _boundary.length(); i++) itemWriteByte(_boundary.c_str()[i]);
      _parseMultipartPostByte(data, last);
    }
  } else if (_multiParseState == EXPECT_FEED2) {
    if (data == '\n') {
      _multiParseState = PARSE_HEADERS;
      _itemIsFile      = false;
    } else {
      _multiParseState = WAIT_FOR_RETURN1;
      itemWriteByte('\r');
      itemWriteByte('\n');
      itemWriteByte('-');
      itemWriteByte('-');
      uint32_t i;
      for (i = 0; i < _boundary.length(); i++) itemWriteByte(_boundary.c_str()[i]);
      itemWriteByte('\r');
      _parseMultipartPostByte(data, last);
    }
  }
}

void AsyncWebServerRequest::_parseLine()
{
  if (_parseState == PARSE_REQ_START) {
    if (!_temp.length()) {
      _parseState = PARSE_REQ_FAIL;
      _client->close();
    } else {
      _parseReqHead();
      _parseState = PARSE_REQ_HEADERS;
    }
    return;
  }

  if (_parseState == PARSE_REQ_HEADERS) {
    if (!_temp.length()) {
      // end of headers
      _server->_attachHandler(this);
      if (_expectingContinue) {
        using namespace std::string_view_literals;
        _client->write("HTTP/1.1 100 Continue\r\n\r\n"sv);
      }
      // check handler for authentication
      if (_contentLength) {
        _parseState = PARSE_REQ_BODY;
      } else {
        _parseState = PARSE_REQ_END;
        if (_handler)
          _handler->handleRequest(this);
        else
          send(501);
      }
    } else {
      _parseReqHeader(_temp);
      _temp.clear();
    }
  }
}

size_t AsyncWebServerRequest::headers() const
{
  return _headers.length();
}

bool AsyncWebServerRequest::hasHeader(std::string_view name) const
{
  for (const auto& h : _headers) {
    if (OpenShock::StringIEquals(h->name(), name)) {
      return true;
    }
  }
  return false;
}

AsyncWebHeader* AsyncWebServerRequest::getHeader(std::string_view name) const
{
  for (const auto& h : _headers) {
    if (OpenShock::StringIEquals(h->name(), name)) {
      return h;
    }
  }
  return nullptr;
}

AsyncWebHeader* AsyncWebServerRequest::getHeader(size_t num) const
{
  auto header = _headers.nth(num);
  return header ? *header : nullptr;
}

size_t AsyncWebServerRequest::params() const
{
  return _params.length();
}

bool AsyncWebServerRequest::hasParam(std::string_view name, bool post, bool file) const
{
  for (const auto& p : _params) {
    if (p->name() == name && p->isPost() == post && p->isFile() == file) {
      return true;
    }
  }
  return false;
}

AsyncWebParameter* AsyncWebServerRequest::getParam(std::string_view name, bool post, bool file) const
{
  for (const auto& p : _params) {
    if (p->name() == name && p->isPost() == post && p->isFile() == file) {
      return p;
    }
  }
  return nullptr;
}

AsyncWebParameter* AsyncWebServerRequest::getParam(size_t num) const
{
  auto param = _params.nth(num);
  return param ? *param : nullptr;
}

void AsyncWebServerRequest::send(AsyncWebServerResponse* response)
{
  _response = response;
  if (_response == NULL) {
    _client->close(true);
    _onDisconnect();
    return;
  }
  if (!_response->_sourceValid()) {
    delete response;
    _response = NULL;
    send(500);
  } else {
    _client->setRxTimeout(0);
    _response->_respond(this);
  }
}

AsyncWebServerResponse* AsyncWebServerRequest::beginResponse(int code, std::string_view contentType, std::string_view content)
{
  return new AsyncBasicResponse(code, contentType, content);
}

AsyncWebServerResponse* AsyncWebServerRequest::beginResponse(FS& fs, std::string_view path, std::string_view contentType, bool download)
{
  String arduPath = OpenShock::StringToArduinoString(path);
  if (fs.exists(arduPath) || (!download && fs.exists(arduPath + ".gz"))) return new AsyncFileResponse(fs, path, contentType, download);
  return NULL;
}

AsyncWebServerResponse* AsyncWebServerRequest::beginResponse(File content, std::string_view path, std::string_view contentType, bool download)
{
  if (content == true) return new AsyncFileResponse(content, path, contentType, download);
  return NULL;
}

AsyncWebServerResponse* AsyncWebServerRequest::beginResponse(Stream& stream, std::string_view contentType, size_t len)
{
  return new AsyncStreamResponse(stream, contentType, len);
}

AsyncWebServerResponse* AsyncWebServerRequest::beginResponse(std::string_view contentType, size_t len, AwsResponseFiller callback)
{
  return new AsyncCallbackResponse(contentType, len, callback);
}

AsyncWebServerResponse* AsyncWebServerRequest::beginChunkedResponse(std::string_view contentType, AwsResponseFiller callback)
{
  if (_version.minor > 0) return new AsyncChunkedResponse(contentType, callback);
  return new AsyncCallbackResponse(contentType, 0, callback);
}

AsyncResponseStream* AsyncWebServerRequest::beginResponseStream(std::string_view contentType, size_t bufferSize)
{
  return new AsyncResponseStream(contentType, bufferSize);
}

void AsyncWebServerRequest::send(int code, std::string_view contentType, std::string_view content)
{
  send(beginResponse(code, contentType, content));
}

void AsyncWebServerRequest::send(FS& fs, std::string_view path, std::string_view contentType, bool download)
{
  String arduPath = OpenShock::StringToArduinoString(path);
  if (fs.exists(arduPath) || (!download && fs.exists(arduPath + ".gz"))) {
    send(beginResponse(fs, path, contentType, download));
  } else
    send(404);
}

void AsyncWebServerRequest::send(File content, std::string_view path, std::string_view contentType, bool download)
{
  if (content == true) {
    send(beginResponse(content, path, contentType, download));
  } else
    send(404);
}

void AsyncWebServerRequest::send(Stream& stream, std::string_view contentType, size_t len)
{
  send(beginResponse(stream, contentType, len));
}

void AsyncWebServerRequest::send(std::string_view contentType, size_t len, AwsResponseFiller callback)
{
  send(beginResponse(contentType, len, callback));
}

void AsyncWebServerRequest::sendChunked(std::string_view contentType, AwsResponseFiller callback)
{
  send(beginChunkedResponse(contentType, callback));
}

void AsyncWebServerRequest::redirect(std::string_view url)
{
  AsyncWebServerResponse* response = beginResponse(302);
  response->addHeader("Location", url);
  send(response);
}

bool AsyncWebServerRequest::hasArg(std::string_view name) const
{
  for (const auto& arg : _params) {
    if (arg->name() == name) {
      return true;
    }
  }
  return false;
}

std::string_view AsyncWebServerRequest::arg(std::string_view name) const
{
  for (const auto& arg : _params) {
    if (arg->name() == name) {
      return arg->value();
    }
  }
  return SharedEmptyString;
}

std::string_view AsyncWebServerRequest::arg(size_t i) const
{
  return getParam(i)->value();
}

std::string_view AsyncWebServerRequest::argName(size_t i) const
{
  return getParam(i)->name();
}

std::string_view AsyncWebServerRequest::pathArg(size_t i) const
{
  auto param = _pathParams.nth(i);
  return param ? **param : SharedEmptyString;
}

std::string_view AsyncWebServerRequest::header(std::string_view name) const
{
  AsyncWebHeader* h = getHeader(name);
  return h ? h->value() : SharedEmptyString;
}

std::string_view AsyncWebServerRequest::header(size_t i) const
{
  AsyncWebHeader* h = getHeader(i);
  return h ? h->value() : SharedEmptyString;
}

std::string_view AsyncWebServerRequest::headerName(size_t i) const
{
  AsyncWebHeader* h = getHeader(i);
  return h ? h->name() : SharedEmptyString;
}

std::string AsyncWebServerRequest::urlDecode(std::string_view text) const
{
  std::size_t len     = text.length();
  std::size_t i       = 0;
  std::string decoded = std::string();
  decoded.reserve(len);  // Allocate the string internal buffer - never longer from source text
  while (i < len) {
    uint8_t decodedChar;
    char encodedChar = text[i++];
    if (encodedChar == '%' && OpenShock::HexUtils::TryParseHexPair(text[i], text[i + 1], decodedChar)) {
      i += 2;
    } else if (encodedChar == '+') {
      decodedChar = ' ';
    } else {
      decodedChar = encodedChar;  // normal ascii char
    }
    decoded.push_back(decodedChar);
  }
  return decoded;
}

const char* AsyncWebServerRequest::methodToString() const
{
  if (_method == HTTP_ANY)
    return "ANY";
  else if (_method & HTTP_GET)
    return "GET";
  else if (_method & HTTP_POST)
    return "POST";
  else if (_method & HTTP_DELETE)
    return "DELETE";
  else if (_method & HTTP_PUT)
    return "PUT";
  else if (_method & HTTP_PATCH)
    return "PATCH";
  else if (_method & HTTP_HEAD)
    return "HEAD";
  else if (_method & HTTP_OPTIONS)
    return "OPTIONS";
  return "UNKNOWN";
}

const char* AsyncWebServerRequest::requestedConnTypeToString() const
{
  switch (_reqconntype) {
    case RCT_NOT_USED:
      return "RCT_NOT_USED";
    case RCT_DEFAULT:
      return "RCT_DEFAULT";
    case RCT_HTTP:
      return "RCT_HTTP";
    case RCT_WS:
      return "RCT_WS";
    case RCT_EVENT:
      return "RCT_EVENT";
    default:
      return "ERROR";
  }
}

bool AsyncWebServerRequest::isExpectedRequestedConnType(RequestedConnectionType erct1, RequestedConnectionType erct2, RequestedConnectionType erct3)
{
  bool res = false;
  if ((erct1 != RCT_NOT_USED) && (erct1 == _reqconntype)) res = true;
  if ((erct2 != RCT_NOT_USED) && (erct2 == _reqconntype)) res = true;
  if ((erct3 != RCT_NOT_USED) && (erct3 == _reqconntype)) res = true;
  return res;
}
