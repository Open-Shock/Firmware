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
#include "external/AsyncWebServer/WebHandlerImpl.h"

#include "Logging.h"

const char* const TAG = "WebHandlers";

AsyncStaticWebHandler::AsyncStaticWebHandler(std::string_view uri, FS& fs, std::string_view path, std::string_view cache_control)
  : _fs(fs)
  , _default_file("index.htm")
  , _cache_control(cache_control)
  , _shared_eTag("")
  , _isDir(false)
{
  // Ensure leading '/'
  if (!OpenShock::StringStartsWith(uri, '/')) uri.remove_prefix(1);
  if (!OpenShock::StringStartsWith(path, '/')) path.remove_prefix(1);

  // Remove the trailing '/' so we can handle default file
  // Notice that root will be "" not "/"
  if (OpenShock::StringEndsWith(uri, '/')) uri.remove_suffix(1);
  if (OpenShock::StringEndsWith(path, '/')) {
    path.remove_suffix(1);

    // If path ends with '/' we assume a hint that this is a directory to improve performance.
    // However - if it does not end with '/' we, can't assume a file, path can still be a directory.
    _isDir = true;
  }

  _uri  = uri;
  _path = path;
}

AsyncStaticWebHandler& AsyncStaticWebHandler::setIsDir(bool isDir)
{
  _isDir = isDir;
  return *this;
}

AsyncStaticWebHandler& AsyncStaticWebHandler::setDefaultFile(std::string_view filename)
{
  _default_file = filename;
  return *this;
}

AsyncStaticWebHandler& AsyncStaticWebHandler::setCacheControl(std::string_view cache_control)
{
  _cache_control = cache_control;
  return *this;
}

AsyncStaticWebHandler& AsyncStaticWebHandler::setSharedEtag(std::string_view etag)
{
  _shared_eTag = etag;
  return *this;
}

bool AsyncStaticWebHandler::canHandle(AsyncWebServerRequest* request)
{
  if (request->method() != HTTP_GET || !OpenShock::StringStartsWith(request->url(), _uri) || !request->isExpectedRequestedConnType(RCT_DEFAULT, RCT_HTTP)) {
    return false;
  }
  if (_getFile(request)) {
    OS_LOGD(TAG, "TRUE");
    return true;
  }

  return false;
}

bool AsyncStaticWebHandler::_getFile(AsyncWebServerRequest* request)
{
  // Remove the found uri
  std::string path(request->url().substr(_uri.length()));

  // We can skip the file check and look for default if request is to the root of a directory or that request path ends with '/'
  bool canSkipFileCheck = (_isDir && path.length() == 0) || OpenShock::StringEndsWith(path, '/');

  path = _path + path;

  // Do we have a file or .gz file
  if (!canSkipFileCheck && _fileExists(request, path)) return true;

  // Can't handle if not default file
  if (_default_file.length() == 0) return false;

  // Try to add default file, ensure there is a trailing '/' ot the path.
  if (!OpenShock::StringEndsWith(path, '/')) path.push_back('/');
  path += _default_file;

  return _fileExists(request, path);
}

#define FILE_IS_REAL(f) (f == true && !f.isDirectory())

bool AsyncStaticWebHandler::_fileExists(AsyncWebServerRequest* request, std::string_view path)
{
  String pathArdu = OpenShock::StringToArduinoString(path);
  String gzipArdu = pathArdu + ".gz";

  request->_tempFile = _fs.open(gzipArdu, "rb");

  if (!FILE_IS_REAL(request->_tempFile)) {
    request->_tempFile = _fs.open(pathArdu, "rb");
    if (!FILE_IS_REAL(request->_tempFile)) {
      return false;
    }
  }

  // Extract the file name from the path and keep it in _tempObject
  size_t pathLen  = path.length();
  char* _tempPath = (char*)malloc(pathLen + 1);
  snprintf(_tempPath, pathLen + 1, "%s", path.data());
  request->_tempObject = (void*)_tempPath;

  return true;
}

void AsyncStaticWebHandler::handleRequest(AsyncWebServerRequest* request)
{
  // Get the filename from request->_tempObject and free it
  std::string filename = (char*)request->_tempObject;
  free(request->_tempObject);
  request->_tempObject = NULL;

  if (request->_tempFile == true) {
    bool canCache = !_cache_control.empty() && !_shared_eTag.empty();
    if (canCache && request->header("If-None-Match") == _shared_eTag) {
      request->_tempFile.close();
      AsyncWebServerResponse* response = new AsyncBasicResponse(304);  // Not modified
      response->addHeader("Cache-Control", _cache_control);
      response->addHeader("ETag", _shared_eTag);
      request->send(response);
    } else {
      AsyncWebServerResponse* response = new AsyncFileResponse(request->_tempFile, filename, {}, false);
      if (canCache) {
        response->addHeader("Cache-Control", _cache_control);
        response->addHeader("ETag", _shared_eTag);
      }
      request->send(response);
    }
  } else {
    request->send(404);
  }
}
