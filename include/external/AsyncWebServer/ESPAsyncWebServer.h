#pragma once

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

#include "Arduino.h"

#include "FS.h"
#include <functional>

#include "HttpRequestMethod.h"
#include "HttpVersion.h"
#include "LinkedList.h"
#include "util/StringUtils.h"

#include <external/AsyncTCP.h>
#include <WiFi.h>

#include <string>
#include <string_view>

class AsyncWebServer;
class AsyncWebServerRequest;
class AsyncWebServerResponse;
class AsyncWebHeader;
class AsyncWebParameter;
class AsyncWebHandler;
class AsyncStaticWebHandler;
class AsyncCallbackWebHandler;
class AsyncResponseStream;

// if this value is returned when asked for data, packet will not be sent and you will be asked for data again
#define RESPONSE_TRY_AGAIN 0xFFFFFFFF

typedef std::function<void(void)> ArDisconnectHandler;

/*
 * PARAMETER :: Chainable object to hold GET/POST and FILE parameters
 * */

class AsyncWebParameter {
private:
  std::string _name;
  std::string _value;
  size_t _size;
  bool _isForm;
  bool _isFile;

public:
  AsyncWebParameter(std::string_view name, std::string_view value, bool form = false, bool file = false, size_t size = 0)
    : _name(name)
    , _value(value)
    , _size(size)
    , _isForm(form)
    , _isFile(file)
  {
  }
  std::string_view name() const { return _name; }
  std::string_view value() const { return _value; }
  size_t size() const { return _size; }
  bool isPost() const { return _isForm; }
  bool isFile() const { return _isFile; }
};

/*
 * HEADER :: Chainable object to hold the headers
 * */

class AsyncWebHeader {
private:
  std::string _name;
  std::string _value;

public:
  AsyncWebHeader(std::string_view name, std::string_view value)
    : _name(name)
    , _value(value)
  {
  }
  AsyncWebHeader(std::string_view data)
    : _name()
    , _value()
  {
    if (data.empty()) return;

    auto pair = OpenShock::StringSplitByFirst(data, ':');

    if (pair.first.empty() || pair.second.empty()) return;

    _name  = pair.first;
    _value = pair.second;
  }
  ~AsyncWebHeader() { }
  std::string_view name() const { return _name; }
  std::string_view value() const { return _value; }
  std::string toString() const { return std::string(_name + ": " + _value + "\r\n"); }
};

/*
 * REQUEST :: Each incoming Client is wrapped inside a Request and both live together until disconnect
 * */

typedef enum {
  RCT_NOT_USED = -1,
  RCT_DEFAULT  = 0,
  RCT_HTTP,
  RCT_WS,
  RCT_EVENT,
  RCT_MAX
} RequestedConnectionType;

typedef std::function<size_t(uint8_t*, size_t, size_t)> AwsResponseFiller;

class AsyncWebServerRequest {
  using File = fs::File;
  using FS   = fs::FS;
  friend class AsyncWebServer;
  friend class AsyncCallbackWebHandler;

private:
  AsyncClient* _client;
  AsyncWebServer* _server;
  AsyncWebHandler* _handler;
  AsyncWebServerResponse* _response;
  ArDisconnectHandler _onDisconnectfn;

  std::string _temp;
  uint8_t _parseState;

  HttpVersion _version;
  HttpRequestMethod _method;
  std::string _url;
  std::string _host;
  std::string _contentType;
  std::string _boundary;
  std::string _authorization;
  RequestedConnectionType _reqconntype;
  bool _isDigest;
  bool _isMultipart;
  bool _isPlainPost;
  bool _expectingContinue;
  size_t _contentLength;
  size_t _parsedLength;

  LinkedList<AsyncWebHeader*> _headers;
  LinkedList<AsyncWebParameter*> _params;
  LinkedList<std::string*> _pathParams;

  uint8_t _multiParseState;
  uint8_t _boundaryPosition;
  size_t _itemStartIndex;
  size_t _itemSize;
  std::string _itemName;
  std::string _itemFilename;
  std::string _itemType;
  std::string _itemValue;
  uint8_t* _itemBuffer;
  size_t _itemBufferIndex;
  bool _itemIsFile;

  void _onPoll();
  void _onAck(size_t len, uint32_t time);
  void _onError(int8_t error);
  void _onTimeout(uint32_t time);
  void _onDisconnect();
  void _onData(void* buf, size_t len);

  void _addParam(AsyncWebParameter*);
  void _addPathParam(const char* param);

  bool _parseReqHead();
  bool _parseReqHeader(std::string_view header);
  void _parseLine();
  void _parsePlainPostChar(uint8_t data);
  void _parseMultipartPostByte(uint8_t data, bool last);
  void _parseQueryParams(std::string_view params);

  void _handleUploadStart();
  void _handleUploadByte(uint8_t data, bool last);
  void _handleUploadEnd();

public:
  File _tempFile;
  void* _tempObject;

  AsyncWebServerRequest(AsyncWebServer*, AsyncClient*);
  ~AsyncWebServerRequest();

  AsyncClient* client() { return _client; }
  HttpVersion version() const { return _version; }
  HttpRequestMethod method() const { return _method; }
  std::string_view url() const { return _url; }
  std::string_view host() const { return _host; }
  std::string_view contentType() const { return _contentType; }
  size_t contentLength() const { return _contentLength; }
  bool multipart() const { return _isMultipart; }
  const char* methodToString() const;
  const char* requestedConnTypeToString() const;
  RequestedConnectionType requestedConnType() const { return _reqconntype; }
  bool isExpectedRequestedConnType(RequestedConnectionType erct1, RequestedConnectionType erct2 = RCT_NOT_USED, RequestedConnectionType erct3 = RCT_NOT_USED);
  void onDisconnect(ArDisconnectHandler fn);

  void setHandler(AsyncWebHandler* handler) { _handler = handler; }

  void redirect(std::string_view url);

  void send(AsyncWebServerResponse* response);
  void send(int code, std::string_view contentType = {}, std::string_view content = {});
  void send(FS& fs, std::string_view path, std::string_view contentType = {}, bool download = false);
  void send(File content, std::string_view path, std::string_view contentType = {}, bool download = false);
  void send(Stream& stream, std::string_view contentType, size_t len);
  void send(std::string_view contentType, size_t len, AwsResponseFiller callback);
  void sendChunked(std::string_view contentType, AwsResponseFiller callback);

  AsyncWebServerResponse* beginResponse(int code, std::string_view contentType = {}, std::string_view content = {});
  AsyncWebServerResponse* beginResponse(FS& fs, std::string_view path, std::string_view contentType = {}, bool download = false);
  AsyncWebServerResponse* beginResponse(File content, std::string_view path, std::string_view contentType = {}, bool download = false);
  AsyncWebServerResponse* beginResponse(Stream& stream, std::string_view contentType, size_t len);
  AsyncWebServerResponse* beginResponse(std::string_view contentType, size_t len, AwsResponseFiller callback);
  AsyncWebServerResponse* beginChunkedResponse(std::string_view contentType, AwsResponseFiller callback);
  AsyncResponseStream* beginResponseStream(std::string_view contentType, size_t bufferSize = 1460);

  size_t headers() const;                       // get header count
  bool hasHeader(std::string_view name) const;  // check if header exists

  AsyncWebHeader* getHeader(std::string_view name) const;
  AsyncWebHeader* getHeader(size_t num) const;

  size_t params() const;  // get arguments count
  bool hasParam(std::string_view name, bool post = false, bool file = false) const;

  AsyncWebParameter* getParam(std::string_view name, bool post = false, bool file = false) const;
  AsyncWebParameter* getParam(size_t num) const;

  size_t args() const { return params(); }            // get arguments count
  std::string_view arg(std::string_view name) const;  // get request argument value by name
  std::string_view arg(size_t i) const;               // get request argument value by number
  std::string_view argName(size_t i) const;           // get request argument name by number
  bool hasArg(std::string_view name) const;           // check if argument exists

  std::string_view pathArg(size_t i) const;

  std::string_view header(std::string_view name) const;  // get request header value by name
  std::string_view header(size_t i) const;               // get request header value by number
  std::string_view headerName(size_t i) const;           // get request header name by number
  std::string urlDecode(std::string_view text) const;
};

/*
 * FILTER :: Callback to filter AsyncWebHandler (done by the Server)
 * */

typedef std::function<bool(AsyncWebServerRequest* request)> ArRequestFilterFunction;

bool ON_STA_FILTER(AsyncWebServerRequest* request);

bool ON_AP_FILTER(AsyncWebServerRequest* request);

/*
 * HANDLER :: One instance can be attached to any Request (done by the Server)
 * */

class AsyncWebHandler {
protected:
  ArRequestFilterFunction _filter;

public:
  AsyncWebHandler() { }
  AsyncWebHandler& setFilter(ArRequestFilterFunction fn)
  {
    _filter = fn;
    return *this;
  }
  bool filter(AsyncWebServerRequest* request) { return _filter == NULL || _filter(request); }
  virtual ~AsyncWebHandler() { }
  virtual bool canHandle(AsyncWebServerRequest* request __attribute__((unused))) { return false; }
  virtual void handleRequest(AsyncWebServerRequest* request __attribute__((unused))) { }
  virtual void handleUpload(
    AsyncWebServerRequest* request __attribute__((unused)),
    std::string_view filename __attribute__((unused)),
    size_t index __attribute__((unused)),
    uint8_t* data __attribute__((unused)),
    size_t len __attribute__((unused)),
    bool final __attribute__((unused))
  )
  {
  }
  virtual void handleBody(AsyncWebServerRequest* request __attribute__((unused)), uint8_t* data __attribute__((unused)), size_t len __attribute__((unused)), size_t index __attribute__((unused)), size_t total __attribute__((unused))) { }
  virtual bool isRequestHandlerTrivial() { return true; }
};

/*
 * RESPONSE :: One instance is created for each Request (attached by the Handler)
 * */

typedef enum {
  RESPONSE_SETUP,
  RESPONSE_HEADERS,
  RESPONSE_CONTENT,
  RESPONSE_WAIT_ACK,
  RESPONSE_END,
  RESPONSE_FAILED
} WebResponseState;

class AsyncWebServerResponse {
protected:
  int _code;
  LinkedList<AsyncWebHeader*> _headers;
  std::string _contentType;
  size_t _contentLength;
  bool _sendContentLength;
  bool _chunked;
  size_t _sentLength;
  size_t _ackedLength;
  size_t _writtenLength;
  WebResponseState _state;
  std::string_view _responseCodeToString(int code);

public:
  AsyncWebServerResponse();
  virtual ~AsyncWebServerResponse();
  virtual void setCode(int code);
  virtual void setContentLength(size_t len);
  virtual void setContentType(std::string_view type);
  virtual void addHeader(std::string_view name, std::string_view value);
  virtual std::string _assembleHead(HttpVersion version);
  virtual bool _started() const;
  virtual bool _finished() const;
  virtual bool _failed() const;
  virtual bool _sourceValid() const;
  virtual void _respond(AsyncWebServerRequest* request);
  virtual size_t _ack(AsyncWebServerRequest* request, size_t len, uint32_t time);
};

/*
 * SERVER :: One instance
 * */

typedef std::function<void(AsyncWebServerRequest* request)> ArRequestHandlerFunction;
typedef std::function<void(AsyncWebServerRequest* request, std::string_view filename, size_t index, uint8_t* data, size_t len, bool final)> ArUploadHandlerFunction;
typedef std::function<void(AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total)> ArBodyHandlerFunction;

class AsyncWebServer {
protected:
  AsyncServer _server;
  LinkedList<AsyncWebHandler*> _handlers;
  AsyncCallbackWebHandler* _catchAllHandler;

public:
  AsyncWebServer(uint16_t port);
  ~AsyncWebServer();

  void begin();
  void end();

#if ASYNC_TCP_SSL_ENABLED
  void onSslFileRequest(AcSSlFileHandler cb, void* arg);
  void beginSecure(const char* cert, const char* private_key_file, const char* password);
#endif

  AsyncWebHandler& addHandler(AsyncWebHandler* handler);
  bool removeHandler(AsyncWebHandler* handler);

  AsyncCallbackWebHandler& on(const char* uri, ArRequestHandlerFunction onRequest);
  AsyncCallbackWebHandler& on(const char* uri, HttpRequestMethod method, ArRequestHandlerFunction onRequest);
  AsyncCallbackWebHandler& on(const char* uri, HttpRequestMethod method, ArRequestHandlerFunction onRequest, ArUploadHandlerFunction onUpload);
  AsyncCallbackWebHandler& on(const char* uri, HttpRequestMethod method, ArRequestHandlerFunction onRequest, ArUploadHandlerFunction onUpload, ArBodyHandlerFunction onBody);

  AsyncStaticWebHandler& serveStatic(const char* uri, fs::FS& fs, const char* path, const char* cache_control = NULL);

  void onNotFound(ArRequestHandlerFunction fn);   // called when handler is not assigned
  void onFileUpload(ArUploadHandlerFunction fn);  // handle file uploads
  void onRequestBody(ArBodyHandlerFunction fn);   // handle posts with plain body content (JSON often transmitted this way as a request)

  void reset();                                   // remove all writers and handlers, with onNotFound/onFileUpload/onRequestBody

  void _handleDisconnect(AsyncWebServerRequest* request);
  void _attachHandler(AsyncWebServerRequest* request);
};

class DefaultHeaders {
  using headers_t = LinkedList<AsyncWebHeader*>;
  headers_t _headers;

  DefaultHeaders()
    : _headers(headers_t([](AsyncWebHeader* h) { delete h; }))
  {
  }

public:
  using ConstIterator = headers_t::ConstIterator;

  void addHeader(std::string_view name, std::string_view value) { _headers.add(new AsyncWebHeader(name, value)); }

  ConstIterator begin() const { return _headers.begin(); }
  ConstIterator end() const { return _headers.end(); }

  DefaultHeaders(DefaultHeaders const&)            = delete;
  DefaultHeaders& operator=(DefaultHeaders const&) = delete;
  static DefaultHeaders& Instance()
  {
    static DefaultHeaders instance;
    return instance;
  }
};

#include "AsyncWebSocket.h"
#include "WebHandlerImpl.h"
#include "WebResponseImpl.h"
