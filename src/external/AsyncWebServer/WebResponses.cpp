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
#include <freertos/FreeRTOS.h>

#include "cbuf.h"
#include "external/AsyncWebServer/ESPAsyncWebServer.h"
#include "external/AsyncWebServer/WebResponseImpl.h"

#include "Convert.h"
#include "util/HexUtils.h"

#include <unordered_map>

/*
 * Abstract Response
 * */
std::string_view AsyncWebServerResponse::_responseCodeToString(int code)
{
  using namespace std::string_view_literals;

  switch (code) {
    case 100:
      return "Continue"sv;
    case 101:
      return "Switching Protocols"sv;
    case 200:
      return "OK"sv;
    case 201:
      return "Created"sv;
    case 202:
      return "Accepted"sv;
    case 203:
      return "Non-Authoritative Information"sv;
    case 204:
      return "No Content"sv;
    case 205:
      return "Reset Content"sv;
    case 206:
      return "Partial Content"sv;
    case 300:
      return "Multiple Choices"sv;
    case 301:
      return "Moved Permanently"sv;
    case 302:
      return "Found"sv;
    case 303:
      return "See Other"sv;
    case 304:
      return "Not Modified"sv;
    case 305:
      return "Use Proxy"sv;
    case 307:
      return "Temporary Redirect"sv;
    case 400:
      return "Bad Request"sv;
    case 401:
      return "Unauthorized"sv;
    case 402:
      return "Payment Required"sv;
    case 403:
      return "Forbidden"sv;
    case 404:
      return "Not Found"sv;
    case 405:
      return "Method Not Allowed"sv;
    case 406:
      return "Not Acceptable"sv;
    case 407:
      return "Proxy Authentication Required"sv;
    case 408:
      return "Request Time-out"sv;
    case 409:
      return "Conflict"sv;
    case 410:
      return "Gone"sv;
    case 411:
      return "Length Required"sv;
    case 412:
      return "Precondition Failed"sv;
    case 413:
      return "Request Entity Too Large"sv;
    case 414:
      return "Request-URI Too Large"sv;
    case 415:
      return "Unsupported Media Type"sv;
    case 416:
      return "Requested range not satisfiable"sv;
    case 417:
      return "Expectation Failed"sv;
    case 500:
      return "Internal Server Error"sv;
    case 501:
      return "Not Implemented"sv;
    case 502:
      return "Bad Gateway"sv;
    case 503:
      return "Service Unavailable"sv;
    case 504:
      return "Gateway Time-out"sv;
    case 505:
      return "HTTP Version not supported"sv;
    default:
      return {};
  }
}

AsyncWebServerResponse::AsyncWebServerResponse()
  : _code(0)
  , _headers(LinkedList<AsyncWebHeader*>([](AsyncWebHeader* h) { delete h; }))
  , _contentType()
  , _contentLength(0)
  , _sendContentLength(true)
  , _chunked(false)
  , _sentLength(0)
  , _ackedLength(0)
  , _writtenLength(0)
  , _state(RESPONSE_SETUP)
{
  for (auto header : DefaultHeaders::Instance()) {
    _headers.add(new AsyncWebHeader(header->name(), header->value()));
  }
}

AsyncWebServerResponse::~AsyncWebServerResponse()
{
  _headers.free();
}

void AsyncWebServerResponse::setCode(int code)
{
  if (_state == RESPONSE_SETUP) _code = code;
}

void AsyncWebServerResponse::setContentLength(size_t len)
{
  if (_state == RESPONSE_SETUP) _contentLength = len;
}

void AsyncWebServerResponse::setContentType(std::string_view type)
{
  if (_state == RESPONSE_SETUP) _contentType = type;
}

void AsyncWebServerResponse::addHeader(std::string_view name, std::string_view value)
{
  _headers.add(new AsyncWebHeader(name, value));
}

std::string AsyncWebServerResponse::_assembleHead(HttpVersion version)
{
  if (version.minor > 0) {
    addHeader("Accept-Ranges", "none");
    if (_chunked) addHeader("Transfer-Encoding", "chunked");
  }
  std::string out = std::string();

  using namespace std::string_view_literals;

  OpenShock::FormatToString(out, "HTTP/%d.%d %d ", version.major, version.minor, _code);
  out += _responseCodeToString(_code);

  if (_sendContentLength) {
    out += "Content-Length: "sv;
    OpenShock::Convert::FromSizeT(_contentLength, out);
    out += "\r\n"sv;
  }

  if (_contentType.length()) {
    out += "Content-Type: "sv;
    out += _contentType;
    out += "\r\n"sv;
  }

  for (const auto& header : _headers) {
    out += header->name();
    out += ": "sv;
    out += header->value();
    out += "\r\n"sv;
  }
  _headers.free();

  out += "\r\n"sv;
  return out;
}

bool AsyncWebServerResponse::_started() const
{
  return _state > RESPONSE_SETUP;
}
bool AsyncWebServerResponse::_finished() const
{
  return _state > RESPONSE_WAIT_ACK;
}
bool AsyncWebServerResponse::_failed() const
{
  return _state == RESPONSE_FAILED;
}
bool AsyncWebServerResponse::_sourceValid() const
{
  return false;
}
void AsyncWebServerResponse::_respond(AsyncWebServerRequest* request)
{
  _state = RESPONSE_END;
  request->client()->close();
}
size_t AsyncWebServerResponse::_ack(AsyncWebServerRequest* request, size_t len, uint32_t time)
{
  (void)request;
  (void)len;
  (void)time;
  return 0;
}

/*
 * std::string/Code Response
 * */
AsyncBasicResponse::AsyncBasicResponse(int code, std::string_view contentType, std::string_view content)
{
  _code        = code;
  _content     = content;
  _contentType = contentType;
  if (_content.length()) {
    _contentLength = _content.length();
    if (!_contentType.length()) _contentType = "text/plain";
  }
  addHeader("Connection", "close");
}

void AsyncBasicResponse::_respond(AsyncWebServerRequest* request)
{
  _state          = RESPONSE_HEADERS;
  std::string out = _assembleHead(request->version());
  size_t space    = request->client()->space();

  if (!_contentLength && space >= out.size()) {
    _writtenLength += request->client()->write(out);
    _state = RESPONSE_WAIT_ACK;
    return;
  }

  if (_contentLength && space >= out.size() + _contentLength) {
    out += _content;
    _writtenLength += request->client()->write(out);
    _state = RESPONSE_WAIT_ACK;
    return;
  }

  if (space && space < out.size()) {
    _content.insert(_content.begin(), out.begin() + space, out.end());
    _contentLength += out.size() - space;
    _writtenLength += request->client()->write(out);
    _state = RESPONSE_CONTENT;
    return;
  }

  if (space > out.size() && space < (out.size() + _contentLength)) {
    size_t shift = space - out.size();
    out += std::string_view(_content).substr(0, shift);
    _content.erase(0, shift);
    _sentLength += shift;
    _writtenLength += request->client()->write(out);
    _state = RESPONSE_CONTENT;
    return;
  }

  _content.insert(_content.begin(), out.begin(), out.end());
  _contentLength += out.size();
  _state = RESPONSE_CONTENT;
}

size_t AsyncBasicResponse::_ack(AsyncWebServerRequest* request, size_t len, uint32_t time)
{
  (void)time;
  _ackedLength += len;
  if (_state == RESPONSE_CONTENT) {
    size_t available = _contentLength - _sentLength;
    size_t space     = request->client()->space();
    // we can fit in this packet
    if (space > available) {
      _writtenLength += request->client()->write(_content.c_str(), available);
      _content.clear();
      _state = RESPONSE_WAIT_ACK;
      return available;
    }
    // send some data, the rest on ack
    size_t nWrite = std::min(_content.size(), space);
    _sentLength += nWrite;
    _writtenLength += request->client()->write(_content.data(), nWrite);
    _content.erase(0, nWrite);
    return nWrite;
  } else if (_state == RESPONSE_WAIT_ACK) {
    if (_ackedLength >= _writtenLength) {
      _state = RESPONSE_END;
    }
  }
  return 0;
}

/*
 * Abstract Response
 * */

AsyncAbstractResponse::AsyncAbstractResponse()
{
}

void AsyncAbstractResponse::_respond(AsyncWebServerRequest* request)
{
  addHeader("Connection", "close");
  _head  = _assembleHead(request->version());
  _state = RESPONSE_HEADERS;
  _ack(request, 0, 0);
}

size_t AsyncAbstractResponse::_ack(AsyncWebServerRequest* request, size_t len, uint32_t time)
{
  (void)time;
  if (!_sourceValid()) {
    _state = RESPONSE_FAILED;
    request->client()->close();
    return 0;
  }

  _ackedLength += len;
  size_t space = request->client()->space();

  if (_state == RESPONSE_HEADERS) {
    if (space < _head.size()) {
      _writtenLength += request->client()->write(_head.data(), space);
      _head.erase(0, space);
      return space;
    }

    space -= _head.size();
    _state = RESPONSE_CONTENT;
  }

  if (_state == RESPONSE_CONTENT) {
    size_t outLen;

    if (_chunked) {
      if (space <= 8) {
        return 0;
      }
      outLen = std::min(space, (size_t)9999);  // Limit to 4 digits
    } else if (_sendContentLength) {
      outLen = std::min(space, _contentLength - _sentLength);
    } else {
      outLen = space;
    }

    uint8_t* buf = (uint8_t*)malloc(outLen);
    if (buf == nullptr) {
      // os_printf("_ack malloc %d failed\n", outLen+headLen);
      return 0;
    }

    size_t readLen = 0;

    if (_chunked) {
      readLen = _fillBuffer(buf + 6, outLen - 8);
      if (readLen == RESPONSE_TRY_AGAIN) {
        free(buf);
        return 0;
      }

      if (sprintf((char*)buf, "%04X", readLen) != 4) {
        // TODO: Handle weird error
      }

      outLen        = 4;
      buf[outLen++] = '\r';
      buf[outLen++] = '\n';
      outLen += readLen;
      buf[outLen++] = '\r';
      buf[outLen++] = '\n';
    } else {
      readLen = _fillBuffer(buf, outLen);
      if (readLen == RESPONSE_TRY_AGAIN) {
        free(buf);
        return 0;
      }
      outLen = readLen;
    }

    _head.clear();

    if (outLen) {
      _writtenLength += request->client()->write((const char*)buf, outLen);
    }

    if (_chunked) {
      _sentLength += readLen;
    } else {
      _sentLength += outLen;
    }

    free(buf);

    if ((_chunked && readLen == 0) || (!_sendContentLength && outLen == 0) || (!_chunked && _sentLength == _contentLength)) {
      _state = RESPONSE_WAIT_ACK;
    }
    return outLen;
  }

  if (_state == RESPONSE_WAIT_ACK) {
    if (!_sendContentLength || _ackedLength >= _writtenLength) {
      _state = RESPONSE_END;
      if (!_chunked && !_sendContentLength) request->client()->close(true);
    }
  }

  return 0;
}

size_t AsyncAbstractResponse::_readDataFromCacheOrContent(uint8_t* data, const size_t len)
{
  // If we have something in cache, copy it to buffer
  const size_t readFromCache = std::min(len, _cache.size());
  if (readFromCache) {
    memcpy(data, _cache.data(), readFromCache);
    _cache.erase(_cache.begin(), _cache.begin() + readFromCache);
  }
  // If we need to read more...
  const size_t needFromFile    = len - readFromCache;
  const size_t readFromContent = _fillBuffer(data + readFromCache, needFromFile);
  return readFromCache + readFromContent;
}

/*
 * File Response
 * */

AsyncFileResponse::~AsyncFileResponse()
{
  if (_content) _content.close();
}

void AsyncFileResponse::_setContentType(std::string_view path)
{
  using namespace std::string_view_literals;

  const std::string_view BINARY_MIME = "application/octet-stream"sv;

  static std::unordered_map<std::string_view, std::string_view> mimeTypes = {
    { ".html"sv,              "text/html"sv},
    {  ".htm"sv,              "text/html"sv},
    {  ".css"sv,               "text/css"sv},
    { ".json"sv,       "application/json"sv},
    {   ".js"sv, "application/javascript"sv},
    {  ".png"sv,              "image/png"sv},
    {  ".gif"sv,              "image/gif"sv},
    {  ".jpg"sv,             "image/jpeg"sv},
    {  ".ico"sv,           "image/x-icon"sv},
    {  ".svg"sv,          "image/svg+xml"sv},
    {  ".eot"sv,               "font/eot"sv},
    { ".woff"sv,              "font/woff"sv},
    {".woff2"sv,             "font/woff2"sv},
    {  ".ttf"sv,               "font/ttf"sv},
    {  ".xml"sv,               "text/xml"sv},
    {  ".pdf"sv,        "application/pdf"sv},
    {  ".zip"sv,        "application/zip"sv},
    {   ".gz"sv,     "application/x-gzip"sv},
    {  ".txt"sv,             "text/plain"sv},
    {  ".bin"sv,                BINARY_MIME},
  };

  size_t lastDot = path.find_last_of('.');
  if (lastDot == std::string_view::npos) {
    _contentType = BINARY_MIME;
    return;
  }

  std::string_view extension = path.substr(lastDot);

  auto it = mimeTypes.find(extension);
  if (it == mimeTypes.end()) {
    _contentType = BINARY_MIME;
    return;
  }

  _contentType = it->second;
}

AsyncFileResponse::AsyncFileResponse(FS& fs, std::string_view path, std::string_view contentType, bool download)
  : AsyncAbstractResponse()
{
  _code             = 200;
  _path             = path;
  String arduPath   = OpenShock::StringToArduinoString(path);
  String arduGzPath = arduPath + ".gz";

  if (!download && !fs.exists(arduPath) && fs.exists(arduGzPath)) {
    _path    = _path + ".gz";
    arduPath = arduGzPath;
    addHeader("Content-Encoding", "gzip");
    _sendContentLength = true;
    _chunked           = false;
  }

  _content       = fs.open(arduPath, "rb");
  _contentLength = _content.size();

  if (contentType == "")
    _setContentType(path);
  else
    _contentType = contentType;

  std::string_view filename = path;
  size_t filenameStart      = path.find_last_of('/');
  if (filenameStart != std::string_view::npos) {
    filename.remove_prefix(filenameStart + 1);
  }

  std::string contentDisp;
  contentDisp.reserve(26 + filename.size());

  using namespace std::string_view_literals;

  if (download) {
    // set filename and force download
    contentDisp.append("attachment; filename=\""sv);
  } else {
    // set filename and force rendering
    contentDisp.append("inline; filename=\""sv);
  }

  contentDisp.append(filename);
  contentDisp.push_back('"');

  addHeader("Content-Disposition", contentDisp);
}

AsyncFileResponse::AsyncFileResponse(File content, std::string_view path, std::string_view contentType, bool download)
  : AsyncAbstractResponse()
{
  _code = 200;
  _path = path;

  using namespace std::string_view_literals;

  if (!download && OpenShock::StringHasSuffix(content.name(), ".gz"sv) && !OpenShock::StringHasSuffix(path, ".gz"sv)) {
    addHeader("Content-Encoding", "gzip");
    _sendContentLength = true;
    _chunked           = false;
  }

  _content       = content;
  _contentLength = _content.size();

  // TODO: The rest of this function is identical to the function above (AsyncFileResponse::AsyncFileResponse(FS&,std::string_view,std::string_view,bool)), merge this logic
  if (contentType == "")
    _setContentType(path);
  else
    _contentType = contentType;

  std::string_view filename = path;
  size_t filenameStart      = path.find_last_of('/');
  if (filenameStart != std::string_view::npos) {
    filename.remove_prefix(filenameStart + 1);
  }

  std::string contentDisp;
  contentDisp.reserve(26 + filename.size());

  using namespace std::string_view_literals;

  if (download) {
    // set filename and force download
    contentDisp.append("attachment; filename=\""sv);
  } else {
    // set filename and force rendering
    contentDisp.append("inline; filename=\""sv);
  }

  contentDisp.append(filename);
  contentDisp.push_back('"');

  addHeader("Content-Disposition", contentDisp);
}

size_t AsyncFileResponse::_fillBuffer(uint8_t* data, size_t len)
{
  return _content.read(data, len);
}

/*
 * Stream Response
 * */

AsyncStreamResponse::AsyncStreamResponse(Stream& stream, std::string_view contentType, size_t len)
  : AsyncAbstractResponse()
{
  _code          = 200;
  _content       = &stream;
  _contentLength = len;
  _contentType   = contentType;
}

size_t AsyncStreamResponse::_fillBuffer(uint8_t* data, size_t len)
{
  size_t available = _content->available();
  size_t outLen    = (available > len) ? len : available;
  size_t i;
  for (i = 0; i < outLen; i++) data[i] = _content->read();
  return outLen;
}

/*
 * Callback Response
 * */

AsyncCallbackResponse::AsyncCallbackResponse(std::string_view contentType, size_t len, AwsResponseFiller callback)
  : AsyncAbstractResponse()
{
  _code          = 200;
  _content       = callback;
  _contentLength = len;
  if (!len) _sendContentLength = false;
  _contentType  = contentType;
  _filledLength = 0;
}

size_t AsyncCallbackResponse::_fillBuffer(uint8_t* data, size_t len)
{
  size_t ret = _content(data, len, _filledLength);
  if (ret != RESPONSE_TRY_AGAIN) {
    _filledLength += ret;
  }
  return ret;
}

/*
 * Chunked Response
 * */

AsyncChunkedResponse::AsyncChunkedResponse(std::string_view contentType, AwsResponseFiller callback)
  : AsyncAbstractResponse()
{
  _code              = 200;
  _content           = callback;
  _contentLength     = 0;
  _contentType       = contentType;
  _sendContentLength = false;
  _chunked           = true;
  _filledLength      = 0;
}

size_t AsyncChunkedResponse::_fillBuffer(uint8_t* data, size_t len)
{
  size_t ret = _content(data, len, _filledLength);
  if (ret != RESPONSE_TRY_AGAIN) {
    _filledLength += ret;
  }
  return ret;
}

/*
 * Response Stream (You can print/write/printf to it, up to the contentLen bytes)
 * */

AsyncResponseStream::AsyncResponseStream(std::string_view contentType, size_t bufferSize)
{
  _code          = 200;
  _contentLength = 0;
  _contentType   = contentType;
  _content       = new cbuf(bufferSize);
}

AsyncResponseStream::~AsyncResponseStream()
{
  delete _content;
}

size_t AsyncResponseStream::_fillBuffer(uint8_t* buf, size_t maxLen)
{
  return _content->read((char*)buf, maxLen);
}

size_t AsyncResponseStream::write(const uint8_t* data, size_t len)
{
  if (_started()) return 0;

  if (len > _content->room()) {
    size_t needed = len - _content->room();
    _content->resizeAdd(needed);
  }
  size_t written = _content->write((const char*)data, len);
  _contentLength += written;
  return written;
}

size_t AsyncResponseStream::write(uint8_t data)
{
  return write(&data, 1);
}
