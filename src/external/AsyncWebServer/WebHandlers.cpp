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

AsyncStaticWebHandler::AsyncStaticWebHandler(const char* uri, FS& fs, const char* path, const char* cache_control)
  : _fs(fs)
  , _uri(uri)
  , _path(path)
  , _default_file("index.htm")
  , _cache_control(cache_control)
  , _shared_eTag("")
{
  // Ensure leading '/'
  if (_uri.length() == 0 || _uri[0] != '/') _uri = "/" + _uri;
  if (_path.length() == 0 || _path[0] != '/') _path = "/" + _path;

  // If path ends with '/' we assume a hint that this is a directory to improve performance.
  // However - if it does not end with '/' we, can't assume a file, path can still be a directory.
  _isDir = _path[_path.length() - 1] == '/';

  // Remove the trailing '/' so we can handle default file
  // Notice that root will be "" not "/"
  if (_uri[_uri.length() - 1] == '/') _uri = _uri.substring(0, _uri.length() - 1);
  if (_path[_path.length() - 1] == '/') _path = _path.substring(0, _path.length() - 1);
}

AsyncStaticWebHandler& AsyncStaticWebHandler::setIsDir(bool isDir)
{
  _isDir = isDir;
  return *this;
}

AsyncStaticWebHandler& AsyncStaticWebHandler::setDefaultFile(const char* filename)
{
  _default_file = String(filename);
  return *this;
}

AsyncStaticWebHandler& AsyncStaticWebHandler::setCacheControl(const char* cache_control)
{
  _cache_control = String(cache_control);
  return *this;
}

AsyncStaticWebHandler& AsyncStaticWebHandler::setSharedEtag(const char* etag)
{
  _shared_eTag = String(etag);
  return *this;
}

bool AsyncStaticWebHandler::canHandle(AsyncWebServerRequest* request)
{
  if (request->method() != HTTP_GET || !request->url().startsWith(_uri) || !request->isExpectedRequestedConnType(RCT_DEFAULT, RCT_HTTP)) {
    return false;
  }
  if (_getFile(request)) {
    DEBUGF("[AsyncStaticWebHandler::canHandle] TRUE\n");
    return true;
  }

  return false;
}

bool AsyncStaticWebHandler::_getFile(AsyncWebServerRequest* request)
{
  // Remove the found uri
  String path = request->url().substring(_uri.length());

  // We can skip the file check and look for default if request is to the root of a directory or that request path ends with '/'
  bool canSkipFileCheck = (_isDir && path.length() == 0) || (path.length() && path[path.length() - 1] == '/');

  path = _path + path;

  // Do we have a file or .gz file
  if (!canSkipFileCheck && _fileExists(request, path)) return true;

  // Can't handle if not default file
  if (_default_file.length() == 0) return false;

  // Try to add default file, ensure there is a trailing '/' ot the path.
  if (path.length() == 0 || path[path.length() - 1] != '/') path += "/";
  path += _default_file;

  return _fileExists(request, path);
}

#define FILE_IS_REAL(f) (f == true && !f.isDirectory())

bool AsyncStaticWebHandler::_fileExists(AsyncWebServerRequest* request, const String& path)
{
  String gzip = path + ".gz";

  request->_tempFile = _fs.open(gzip, "rb");

  if (!FILE_IS_REAL(request->_tempFile)) {
    request->_tempFile = _fs.open(path, "rb");
    if (!FILE_IS_REAL(request->_tempFile)) {
      return false;
    }
  }

  // Extract the file name from the path and keep it in _tempObject
  size_t pathLen  = path.length();
  char* _tempPath = (char*)malloc(pathLen + 1);
  snprintf(_tempPath, pathLen + 1, "%s", path.c_str());
  request->_tempObject = (void*)_tempPath;

  return true;
}

uint8_t AsyncStaticWebHandler::_countBits(const uint8_t value) const
{
  uint8_t w = value;
  uint8_t n;
  for (n = 0; w != 0; n++) w &= w - 1;
  return n;
}

void AsyncStaticWebHandler::handleRequest(AsyncWebServerRequest* request)
{
  // Get the filename from request->_tempObject and free it
  String filename = String((char*)request->_tempObject);
  free(request->_tempObject);
  request->_tempObject = NULL;

  if (request->_tempFile == true) {
    bool canCache = !_cache_control.isEmpty() && !_shared_eTag.isEmpty();
    if (canCache && request->header("If-None-Match").equals(_shared_eTag)) {
      request->_tempFile.close();
      AsyncWebServerResponse* response = new AsyncBasicResponse(304);  // Not modified
      response->addHeader("Cache-Control", _cache_control);
      response->addHeader("ETag", _shared_eTag);
      request->send(response);
    } else {
      AsyncWebServerResponse* response = new AsyncFileResponse(request->_tempFile, filename, String(), false);
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
