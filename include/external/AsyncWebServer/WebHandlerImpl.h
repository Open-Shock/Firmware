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

#include <string>

#include "stddef.h"
#include <time.h>

class AsyncStaticWebHandler : public AsyncWebHandler {
  using File = fs::File;
  using FS   = fs::FS;

private:
  bool _getFile(AsyncWebServerRequest* request);
  bool _fileExists(AsyncWebServerRequest* request, std::string_view path);

protected:
  FS _fs;
  std::string _uri;
  std::string _path;
  std::string _default_file;
  std::string _cache_control;
  std::string _shared_eTag;
  bool _isDir;

public:
  AsyncStaticWebHandler(const char* uri, FS& fs, const char* path, const char* cache_control);
  virtual bool canHandle(AsyncWebServerRequest* request) override final;
  virtual void handleRequest(AsyncWebServerRequest* request) override final;
  AsyncStaticWebHandler& setIsDir(bool isDir);
  AsyncStaticWebHandler& setDefaultFile(const char* filename);
  AsyncStaticWebHandler& setCacheControl(std::string_view cache_control);
  AsyncStaticWebHandler& setSharedEtag(std::string_view etag);
};

class AsyncCallbackWebHandler : public AsyncWebHandler {
private:
protected:
  std::string _uri;
  HttpRequestMethod _method;
  ArRequestHandlerFunction _onRequest;
  ArUploadHandlerFunction _onUpload;
  ArBodyHandlerFunction _onBody;
  bool _isRegex;

public:
  AsyncCallbackWebHandler()
    : _uri()
    , _method(HTTP_ANY)
    , _onRequest(NULL)
    , _onUpload(NULL)
    , _onBody(NULL)
    , _isRegex(false)
  {
  }
  void setUri(std::string_view uri)
  {
    _uri     = uri;
    _isRegex = uri.length() > 1 && uri.front() == '^' && uri.back() == '$';
  }
  void setMethod(HttpRequestMethod method) { _method = method; }
  void onRequest(ArRequestHandlerFunction fn) { _onRequest = fn; }
  void onUpload(ArUploadHandlerFunction fn) { _onUpload = fn; }
  void onBody(ArBodyHandlerFunction fn) { _onBody = fn; }

  virtual bool canHandle(AsyncWebServerRequest* request) override final
  {
    if (!_onRequest) return false;

    if (!(_method & request->method())) return false;

    using namespace std::string_view_literals;

    std::string_view uri        = _uri;
    std::string_view requestUrl = request->url();

    // Match 1 ??
    if (OpenShock::StringStartsWith(uri, "/*."sv)) {
      uri = uri.substr(uri.find_last_of('.'));

      return OpenShock::StringEndsWith(requestUrl, uri);
    }

    // Match 2 ????
    if (OpenShock::StringEndsWith(uri, '*')) {
      uri.remove_suffix(1);

      return OpenShock::StringStartsWith(requestUrl, uri);
    }

    // Huh ??????
    if (uri.empty() || uri == requestUrl) {
      return true;
    }

    // Way to check if requestUrl ends with uri + "/" without any allocations
    return requestUrl.length() > uri.length() && requestUrl[uri.length()] == '/' && OpenShock::StringStartsWith(requestUrl, uri);
  }

  virtual void handleRequest(AsyncWebServerRequest* request) override final
  {
    if (_onRequest)
      _onRequest(request);
    else
      request->send(500);
  }
  virtual void handleUpload(AsyncWebServerRequest* request, std::string_view filename, size_t index, uint8_t* data, size_t len, bool final) override final
  {
    if (_onUpload) _onUpload(request, filename, index, data, len, final);
  }
  virtual void handleBody(AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) override final
  {
    if (_onBody) _onBody(request, data, len, index, total);
  }
  virtual bool isRequestHandlerTrivial() override final { return _onRequest ? false : true; }
};
