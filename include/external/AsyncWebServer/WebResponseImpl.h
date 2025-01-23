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

#ifdef Arduino_h
// arduino is not compatible with std::vector
#undef min
#undef max
#endif
#include <vector>
// It is possible to restore these defines, but one can use _min and _max instead. Or std::min, std::max.

class AsyncBasicResponse : public AsyncWebServerResponse {
private:
  std::string _content;

public:
  AsyncBasicResponse(int code, std::string_view contentType = {}, std::string_view content = {});
  void _respond(AsyncWebServerRequest* request);
  size_t _ack(AsyncWebServerRequest* request, size_t len, uint32_t time);
  bool _sourceValid() const { return true; }
};

class AsyncAbstractResponse : public AsyncWebServerResponse {
private:
  std::string _head;
  // Data is inserted into cache at begin().
  // This is inefficient with vector, but if we use some other container,
  // we won't be able to access it as contiguous array of bytes when reading from it,
  // so by gaining performance in one place, we'll lose it in another.
  std::vector<uint8_t> _cache;
  size_t _readDataFromCacheOrContent(uint8_t* data, const size_t len);

public:
  AsyncAbstractResponse();
  void _respond(AsyncWebServerRequest* request);
  size_t _ack(AsyncWebServerRequest* request, size_t len, uint32_t time);
  bool _sourceValid() const { return false; }
  virtual size_t _fillBuffer(uint8_t* buf __attribute__((unused)), size_t maxLen __attribute__((unused))) { return 0; }
};

#ifndef TEMPLATE_PLACEHOLDER
#define TEMPLATE_PLACEHOLDER '%'
#endif

#define TEMPLATE_PARAM_NAME_LENGTH 32
class AsyncFileResponse : public AsyncAbstractResponse {
  using File = fs::File;
  using FS   = fs::FS;

private:
  File _content;
  std::string _path;
  void _setContentType(std::string_view path);

public:
  AsyncFileResponse(FS& fs, std::string_view path, std::string_view contentType = {}, bool download = false);
  AsyncFileResponse(File content, std::string_view path, std::string_view contentType = {}, bool download = false);
  ~AsyncFileResponse();
  bool _sourceValid() const { return !!(_content); }
  virtual size_t _fillBuffer(uint8_t* buf, size_t maxLen) override;
};

class AsyncStreamResponse : public AsyncAbstractResponse {
private:
  Stream* _content;

public:
  AsyncStreamResponse(Stream& stream, std::string_view contentType, size_t len);
  bool _sourceValid() const { return !!(_content); }
  virtual size_t _fillBuffer(uint8_t* buf, size_t maxLen) override;
};

class AsyncCallbackResponse : public AsyncAbstractResponse {
private:
  AwsResponseFiller _content;
  size_t _filledLength;

public:
  AsyncCallbackResponse(std::string_view contentType, size_t len, AwsResponseFiller callback);
  bool _sourceValid() const { return !!(_content); }
  virtual size_t _fillBuffer(uint8_t* buf, size_t maxLen) override;
};

class AsyncChunkedResponse : public AsyncAbstractResponse {
private:
  AwsResponseFiller _content;
  size_t _filledLength;

public:
  AsyncChunkedResponse(std::string_view contentType, AwsResponseFiller callback);
  bool _sourceValid() const { return !!(_content); }
  virtual size_t _fillBuffer(uint8_t* buf, size_t maxLen) override;
};

class cbuf;

class AsyncResponseStream
  : public AsyncAbstractResponse
  , public Print {
private:
  cbuf* _content;

public:
  AsyncResponseStream(std::string_view contentType, size_t bufferSize);
  ~AsyncResponseStream();
  bool _sourceValid() const { return (_state < RESPONSE_END); }
  virtual size_t _fillBuffer(uint8_t* buf, size_t maxLen) override;
  size_t write(const uint8_t* data, size_t len);
  size_t write(uint8_t data);
  using Print::write;
};
