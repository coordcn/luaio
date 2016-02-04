local color = require('color')
local http_native = require('http_native')
local fs = require('fs')
local ReadBuffer = require('read_buffer')
local ERRNO = require('errno')

local FOLDER = "http_parser/"

function main()
  CURL_GET()
  FIREFOX_GET()
  DUMBFUCK()
  FRAGMENT_IN_URI()

  print(color.green('test_http_parser ok'))
end

function TEST_REQ(path, data, buffer_size, request, headers)
  local fd = fs.open(path, 'w')
  fs.write(fd, data)
  fs.close(fd)
  fd = fs.open(path)
  local buffer = ReadBuffer.new(buffer_size)
  local size = fs.read(fd, buffer)
  fs.close(fd)
  local parser = http_native.new_parser()
  local method, major, minor, url, err = parser:parse_request_line(buffer)
  assert(err == http_native.OK)
  assert(method == request.method)
  assert(major == request.major)
  assert(minor == request.minor)
  assert(url.schema == request.schema)
  assert(url.auth == request.auth)
  assert(url.host == request.host)
  assert(url.port == request.port)
  assert(url.path == request.path)
  assert(url.query == request.query)
  print(url.query)
  assert(url.hash == request.hash)
  print(url.hash)
  local ret, n, err = parser:parse_headers(buffer)
  assert(n == request.nheader)
  assert(err == http_native.OK)
  for i = 1, n do
    assert(ret[i] == headers[i])
  end
end

function CURL_GET()
  local path = FOLDER .. 'CURL_GET.txt'
  local data = {
    "GET /test HTTP/1.1\r\n",
    "User-Agent: curl/7.18.0 (i486-pc-linux-gnu) libcurl/7.18.0 OpenSSL/0.9.8g zlib/1.2.3.3 libidn/1.1\r\n",
    "Host: 0.0.0.0=5000\r\n",
    "Accept: */*\r\n",
    "\r\n"
  }

  local request = {
    method = http_native.GET,
    major = 1,
    minor = 1,
    schema = nil,
    auth = nil,
    host = nil,
    port = nil,
    path = "/test",
    query = nil,
    hash = nil,
    nheader = 6,
  }

  local headers = {
    "User-Agent",
    "curl/7.18.0 (i486-pc-linux-gnu) libcurl/7.18.0 OpenSSL/0.9.8g zlib/1.2.3.3 libidn/1.1",
    "Host",
    "0.0.0.0=5000",
    "Accept",
    "*/*",
  }

  TEST_REQ(path, data, 512, request, headers)
end

function FIREFOX_GET()
  local path = FOLDER .. 'FIREFOX_GET.txt'
  local data = {
    "GET /favicon.ico HTTP/1.1\r\n",
    "Host: 0.0.0.0=5000\r\n",
    "User-Agent: Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.9) Gecko/2008061015 Firefox/3.0\r\n",
    "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n",
    "Accept-Language: en-us,en;q=0.5\r\n",
    "Accept-Encoding: gzip,deflate\r\n",
    "Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n",
    "Keep-Alive: 300\r\n",
    "Connection: keep-alive\r\n",
    "\r\n"
  }

  local request = {
    method = http_native.GET,
    major = 1,
    minor = 1,
    schema = nil,
    auth = nil,
    host = nil,
    port = nil,
    path = "/favicon.ico",
    query = nil,
    hash = nil,
    nheader = 16,
  }

  local headers = {
    "Host",
    "0.0.0.0=5000",
    "User-Agent",
    "Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.9) Gecko/2008061015 Firefox/3.0",
    "Accept",
    "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8",
    "Accept-Language",
    "en-us,en;q=0.5",
    "Accept-Encoding",
    "gzip,deflate",
    "Accept-Charset",
    "ISO-8859-1,utf-8;q=0.7,*;q=0.7",
    "Keep-Alive",
    "300",
    "Connection",
    "keep-alive"
  }

  TEST_REQ(path, data, 512, request, headers)
end

function DUMBFUCK()
  local path = FOLDER .. 'DUMBFUCK.txt'
  local data = {
    "GET /dumbfuck HTTP/1.1\r\n",
    "aaaaaaaaaaaaa:++++++++++\r\n",
    "\r\n"
  }

  local request = {
    method = http_native.GET,
    major = 1,
    minor = 1,
    schema = nil,
    auth = nil,
    host = nil,
    port = nil,
    path = "/dumbfuck",
    query = nil,
    hash = nil,
    nheader = 2,
  }

  local headers = {
    "aaaaaaaaaaaaa",
    "++++++++++"
  }

  TEST_REQ(path, data, 512, request, headers)
end

function FRAGMENT_IN_URI()
  local path = FOLDER .. 'FRAGMENT_IN_URI.txt'
  local data = {
    "GET /forums/1/topics/2375?page=1#posts-17408 HTTP/1.1\r\n",
    "\r\n"
  }

  local request = {
    method = http_native.GET,
    major = 1,
    minor = 1,
    schema = nil,
    auth = nil,
    host = nil,
    port = nil,
    path = "/forums/1/topics/2375",
    query = "page=1",
    hash = "posts-17408",
    nheader = 0,
  }

  local headers = {
  }

  TEST_REQ(path, data, 512, request, headers)
end

function GET_NO_HEADERS_NO_BODY()
  local path = FOLDER .. 'GET_NO_HEADERS_NO_BODY.txt'
  local data = {
    "GET /get_no_headers_no_body/world HTTP/1.1\r\n",
    "\r\n"
  }

  local request = {
    method = http_native.GET,
    major = 1,
    minor = 1,
    schema = nil,
    auth = nil,
    host = nil,
    port = nil,
    path = "/get_no_headers_no_body/world",
    query = nil,
    hash = nil,
    nheader = 0,
  }

  local headers = {
  }

  TEST_REQ(path, data, 512, request, headers)
end

function GET_ONE_HEADERS_NO_BODY()
  local path = FOLDER .. 'GET_ONE_HEADERS_NO_BODY.txt'
  local data = {
    "GET /get_one_headers_no_body/world HTTP/1.1\r\n",
    "Accept: */*\r\n",
    "\r\n"
  }

  local request = {
    method = http_native.GET,
    major = 1,
    minor = 1,
    schema = nil,
    auth = nil,
    host = nil,
    port = nil,
    path = "/get_one_headers_no_body/world",
    query = nil,
    hash = nil,
    nheader = 2,
  }

  local headers = {
    "Accept",
    "*/*"
  }

  TEST_REQ(path, data, 512, request, headers)
end

function GET_FUNKY_CONTENT_LENGTH()
  local path = FOLDER .. 'GET_FUNKY_CONTENT_LENGTH.txt'
  local data = {
    "GET /get_funky_content_length_body_hello HTTP/1.0\r\n",
    "conTENT-Length: 5\r\n",
    "\r\n",
    "HELLO"
  }

  local request = {
    method = http_native.GET,
    major = 1,
    minor = 1,
    schema = nil,
    auth = nil,
    host = nil,
    port = nil,
    path = "/get_funky_content_length_body_hello",
    query = nil,
    hash = nil,
    nheader = 2,
    body = "HELLO"
  }

  local headers = {
    "conTENT-Length",
    "5"
  }

  TEST_REQ(path, data, 512, request, headers)
end

return main
