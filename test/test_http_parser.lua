-- @refer: https://github.com/nodejs/http-parser test.c
local color = require('color')
local http_native = require('http_native')
local fs = require('fs')
local ReadBuffer = require('read_buffer')
local ERRNO = require('errno')

local DIR = "http_parser/"
fs.mkdir(DIR)

local PRINT_RESULT = true
local BUFFER_SIZE = 512

function TEST_REQ(name, show, data, buffer_size, request, vheaders, vcookies)
  if show then
    print(color.blue('TESTING: ' .. name))
  end

  local path = DIR .. name .. '.txt'
  local fd = fs.open(path, 'w')
  fs.write(fd, data)
  fs.close(fd)

  fd = fs.open(path)
  local buffer = ReadBuffer.new(buffer_size)
  local size = fs.read(fd, buffer)

  local parser = http_native.new_parser()

  if request then
    local method, major, minor, url, err
    while true do
      method, major, minor, url, err = parser:parse_request_line(buffer)

      if err >= 0 or err == http_native.ERROR then break end

      if err == http_native.AGAIN then 
        size = fs.read(fd, buffer)
      end
    end

    if show then 
      print('parse_request error: ' .. err)
      print('method: ' .. method)
      print('major: ' .. major)
      print('minor: ' .. minor)
      if url.schema then print('url.schema: ' .. url.schema) end
      if url.auth then print('url.auth: ' .. url.auth) end
      if url.host then print('url.host: ' .. url.host) end
      if url.port then print('url.port: ' .. url.port) end
      if url.path then print('url.path: ' .. url.path) end
      if url.query then print('url.query: ' .. url.query) end
      if url.hash then print('url.hash: ' .. url.hash) end
    end

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
    assert(url.hash == request.hash)
  end

  if vheaders or vcookies then
    local headers = {}
    local cookies = {}
    local err
    while true do
      err = parser:parse_headers(buffer, headers, cookies)
      if err ~= http_native.AGAIN then break end
      size = fs.read(fd, buffer)
    end

    if show then print('parse_headers error: ' .. err) end
    assert(err == http_native.OK)

    if vheaders then
      for k,v in pairs(headers) do
        if show then print(k .. ': ' .. v) end
        assert(headers[k] == vheaders[k])
      end
    end

    if vcookies then
      for i = 1, #vcookies do
        if show then print('cookie[' .. i .. ']: ' .. cookies[i]) end
        assert(cookies[i] == vcookies[i])
      end
    end
  end

  fs.close(fd)
end

function CURL_GET()
  local name = "CURL_GET"
  local data = {
    "GET /test HTTP/1.1\r\n",
    "User-Agent: curl/7.18.0 (i486-pc-linux-gnu) libcurl/7.18.0 OpenSSL/0.9.8g zlib/1.2.3.3 libidn/1.1\r\n",
    "Host: 0.0.0.0=5000\r\n",
    "Accept: */*\r\n",
    "\r\n"
  }

  local request = {
    should_keep_alive = true,
    message_complete_on_eof = false,
    method = http_native.GET,
    major = 1,
    minor = 1,
    schema = nil,
    auth = nil,
    host = nil,
    port = nil,
    path = "/test",
    query = nil,
    hash = nil
  }

  local headers = {
    ["user-agent"] = "curl/7.18.0 (i486-pc-linux-gnu) libcurl/7.18.0 OpenSSL/0.9.8g zlib/1.2.3.3 libidn/1.1",
    ["host"] = "0.0.0.0=5000",
    ["accept"] = "*/*"
  }

  local show = PRINT_RESULT
  local buffer_size = BUFFER_SIZE
  TEST_REQ(name, show, data, buffer_size, request, headers)
end

function FIREFOX_GET()
  local name = "FIREFOX_GET"
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
    should_keep_alive = true,
    message_complete_on_eof = false,
    method = http_native.GET,
    major = 1,
    minor = 1,
    schema = nil,
    auth = nil,
    host = nil,
    port = nil,
    path = "/favicon.ico",
    query = nil,
    hash = nil
  }

  local headers = {
    ["host"] = "0.0.0.0=5000",
    ["user-agent"] = "Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.9) Gecko/2008061015 Firefox/3.0",
    ["accept"] = "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8",
    ["accept-language"] = "en-us,en;q=0.5",
    ["accept-encoding"] = "gzip,deflate",
    ["accept-charset"] = "ISO-8859-1,utf-8;q=0.7,*;q=0.7",
    ["keep-alive"] = "300",
    ["connection"] = "keep-alive"
  }

  local show = PRINT_RESULT
  local buffer_size = BUFFER_SIZE
  TEST_REQ(name, show, data, buffer_size, request, headers)
end

function DUMBFUCK()
  local name = "DUMBFUCK"
  local data = {
    "GET /dumbfuck HTTP/1.1\r\n",
    "aaaaaaaaaaaaa:++++++++++\r\n",
    "\r\n"
  }

  local request = {
    should_keep_alive = true,
    message_complete_on_eof = false,
    method = http_native.GET,
    major = 1,
    minor = 1,
    schema = nil,
    auth = nil,
    host = nil,
    port = nil,
    path = "/dumbfuck",
    query = nil,
    hash = nil
  }

  local headers = {
    ["aaaaaaaaaaaaa"] = "++++++++++"
  }

  local show = PRINT_RESULT
  local buffer_size = BUFFER_SIZE
  TEST_REQ(name, show, data, buffer_size, request, headers)
end

function FRAGMENT_IN_URI()
  local name = "FRAGMENT_IN_URI"
  local data = {
    "GET /forums/1/topics/2375?page=1#posts-17408 HTTP/1.1\r\n",
    "\r\n"
  }

  local request = {
    should_keep_alive = true,
    message_complete_on_eof = false,
    method = http_native.GET,
    major = 1,
    minor = 1,
    schema = nil,
    auth = nil,
    host = nil,
    port = nil,
    path = "/forums/1/topics/2375",
    query = "page=1",
    hash = "posts-17408"
  }

  local headers = {}

  local show = PRINT_RESULT
  local buffer_size = BUFFER_SIZE
  TEST_REQ(name, show, data, buffer_size, request, headers)
end

function GET_NO_HEADERS_NO_BODY()
  local name = "GET_NO_HEADERS_NO_BODY"
  local data = {
    "GET /get_no_headers_no_body/world HTTP/1.1\r\n",
    "\r\n"
  }

  local request = {
    should_keep_alive = true,
    message_complete_on_eof = false, -- would need Connection: close
    method = http_native.GET,
    major = 1,
    minor = 1,
    schema = nil,
    auth = nil,
    host = nil,
    port = nil,
    path = "/get_no_headers_no_body/world",
    query = nil,
    hash = nil
  }

  local headers = {}

  local show = PRINT_RESULT
  local buffer_size = BUFFER_SIZE
  TEST_REQ(name, show, data, buffer_size, request, headers)
end

function GET_ONE_HEADER_NO_BODY()
  local name = "GET_ONE_HEADER_NO_BODY"
  local data = {
    "GET /get_one_headers_no_body/world HTTP/1.1\r\n",
    "Accept: */*\r\n",
    "\r\n"
  }

  local request = {
    should_keep_alive = true,
    message_complete_on_eof = false, -- would need Connection: close
    method = http_native.GET,
    major = 1,
    minor = 1,
    schema = nil,
    auth = nil,
    host = nil,
    port = nil,
    path = "/get_one_headers_no_body/world",
    query = nil,
    hash = nil
  }

  local headers = {
    ["accept"] = "*/*"
  }

  local show = PRINT_RESULT
  local buffer_size = BUFFER_SIZE
  TEST_REQ(name, show, data, buffer_size, request, headers)
end

function GET_FUNKY_CONTENT_LENGTH()
  local name = "GET_FUNKY_CONTENT_LENGTH"
  local data = {
    "GET /get_funky_content_length_body_hello HTTP/1.0\r\n",
    "conTENT-Length: 5\r\n",
    "\r\n",
    "HELLO"
  }

  local request = {
    should_keep_alive = false,
    message_complete_on_eof = false,
    method = http_native.GET,
    major = 1,
    minor = 0,
    schema = nil,
    auth = nil,
    host = nil,
    port = nil,
    path = "/get_funky_content_length_body_hello",
    query = nil,
    hash = nil,
    body = "HELLO"
  }

  local headers = {
    ["content-length"] = "5"
  }

  local show = PRINT_RESULT
  local buffer_size = BUFFER_SIZE
  TEST_REQ(name, show, data, buffer_size, request, headers)
end

function POST_IDENTITY_BODY_WORLD()
  local name = "POST_IDENTITY_BODY_WORLD"
  local data = {
    "POST /post_identity_body_world?q=search#hey HTTP/1.1\r\n",
    "Accept: */*\r\n",
    "Transfer-Encoding: identity\r\n",
    "Content-Length: 5\r\n",
    "\r\n",
    "World"
  }

  local request = {
    should_keep_alive = true,
    message_complete_on_eof = false,
    method = http_native.POST,
    major = 1,
    minor = 1,
    schema = nil,
    auth = nil,
    host = nil,
    port = nil,
    path = "/post_identity_body_world",
    query = "q=search",
    hash = "hey",
    body = "World"
  }

  local headers = {
    ["accept"]  = "*/*",
    ["transfer-encoding"] = "identity",
    ["content-length"] = "5"
  }

  local show = PRINT_RESULT
  local buffer_size = BUFFER_SIZE
  TEST_REQ(name, show, data, buffer_size, request, headers)
end

function POST_CHUNKED_ALL_YOUR_BASE()
  local name = "POST_CHUNKED_ALL_YOUR_BASE"
  local data = {
    "POST /post_chunked_all_your_base HTTP/1.1\r\n",
    "Transfer-Encoding: chunked\r\n",
    "\r\n",
    "1e\r\nall your base are belong to us\r\n",
    "0\r\n",
    "\r\n"
  }

  local request = {
    should_keep_alive = true,
    message_complete_on_eof = false,
    method = http_native.POST,
    major = 1,
    minor = 1,
    schema = nil,
    auth = nil,
    host = nil,
    port = nil,
    path = "/post_chunked_all_your_base",
    query = nil,
    hash = nil
  }

  local headers = {
    ["transfer-encoding"] = "chunked"
  }

  local show = PRINT_RESULT
  local buffer_size = BUFFER_SIZE
  TEST_REQ(name, show, data, buffer_size, request, headers)
end

-- TODO not support
function TWO_CHUNKS_MULT_ZERO_END()
  local name = "TWO_CHUNKS_MULT_ZERO_END"
  local data = {
    "POST /two_chunks_mult_zero_end HTTP/1.1\r\n",
    "Transfer-Encoding: chunked\r\n",
    "\r\n",
    "5\r\nhello\r\n",
    "6\r\n world\r\n",
    "000\r\n",
    "\r\n"
  }

  local request = {
    should_keep_alive = true,
    message_complete_on_eof = false,
    method = http_native.POST,
    major = 1,
    minor = 1,
    schema = nil,
    auth = nil,
    host = nil,
    port = nil,
    path = "/two_chunks_mult_zero_end",
    query = nil,
    hash = nil
  }

  local headers = {
    ["transfer-encoding"] = "chunked"
  }

  local show = PRINT_RESULT
  local buffer_size = BUFFER_SIZE
  TEST_REQ(name, show, data, buffer_size, request, headers)
end

-- TODO not support 
-- chunked with trailing headers. blech.
function CHUNKED_W_TRAILING_HEADERS()
  local name = "CHUNKED_W_TRAILING_HEADERS"
  local data = {
    "POST /chunked_w_trailing_headers HTTP/1.1\r\n",
    "Transfer-Encoding: chunked\r\n",
    "\r\n",
    "5\r\nhello\r\n",
    "6\r\n world\r\n",
    "0\r\n",
    "Vary: *\r\n",
    "Content-Type: text/plain\r\n",
    "\r\n"
  }

  local request = {
    should_keep_alive = true,
    message_complete_on_eof = false,
    method = http_native.POST,
    major = 1,
    minor = 1,
    schema = nil,
    auth = nil,
    host = nil,
    port = nil,
    path = "/chunked_w_trailing_headers",
    query = nil,
    hash = nil
  }

  local headers = {
    ["transfer-encoding"] = "chunked"
  }

  local show = PRINT_RESULT
  local buffer_size = BUFFER_SIZE
  TEST_REQ(name, show, data, buffer_size, request, headers)
end

-- TODO not support
-- with bullshit after the length
function CHUNKED_W_BULLSHIT_AFTER_LENGTH()
  local name = "CHUNKED_W_BULLSHIT_AFTER_LENGTH"
  local data = {
    "POST /chunked_w_bullshit_after_length HTTP/1.1\r\n",
    "Transfer-Encoding: chunked\r\n",
    "\r\n",
    "5; ihatew3;whatthefuck=aretheseparametersfor\r\nhello\r\n",
    "6; blahblah; blah\r\n world\r\n",
    "0\r\n",
    "\r\n"
  }

  local request = {
    should_keep_alive = true,
    message_complete_on_eof = false,
    method = http_native.POST,
    major = 1,
    minor = 1,
    schema = nil,
    auth = nil,
    host = nil,
    port = nil,
    path = "/chunked_w_bullshit_after_length",
    query = nil,
    hash = nil
  }

  local headers = {
    ["transfer-encoding"] = "chunked"
  }

  local show = PRINT_RESULT
  local buffer_size = BUFFER_SIZE
  TEST_REQ(name, show, data, buffer_size, request, headers)
end

function WITH_QUOTES()
  local name = "WITH_QUOTES"
  local data = "GET /with_\"stupid\"_quotes?foo=\"bar\" HTTP/1.1\r\n\r\n"

  local request = {
    should_keep_alive = true,
    message_complete_on_eof = false,
    method = http_native.GET,
    major = 1,
    minor = 1,
    schema = nil,
    auth = nil,
    host = nil,
    port = nil,
    path = "/with_\"stupid\"_quotes",
    query = "foo=\"bar\"",
    hash = nil
  }

  local headers = {}

  local show = PRINT_RESULT
  local buffer_size = BUFFER_SIZE
  TEST_REQ(name, show, data, buffer_size, request, headers)
end

-- The server receiving this request SHOULD NOT wait for EOF
-- to know that content-length == 0.
function APACHEBENCH_GET()
  local name = "APACHEBENCH_GET"
  local data = {
    "GET /test HTTP/1.0\r\n",
    "Host: 0.0.0.0:5000\r\n",
    "User-Agent: ApacheBench/2.3\r\n",
    "Accept: */*\r\n\r\n"
  }

  local request = {
    should_keep_alive = false,
    message_complete_on_eof = false,
    method = http_native.GET,
    major = 1,
    minor = 0,
    schema = nil,
    auth = nil,
    host = nil,
    port = nil,
    path = "/test",
    query = nil,
    hash = nil
  }

  local headers = {
    ["host"] = "0.0.0.0:5000",
    ["user-agent"] = "ApacheBench/2.3",
    ["accept"] = "*/*"
  }

  local show = PRINT_RESULT
  local buffer_size = BUFFER_SIZE
  TEST_REQ(name, show, data, buffer_size, request, headers)
end

-- Some clients include '?' characters in query strings.
function QUERY_URL_WITH_QUESTION_MARK_GET()
  local name = "QUERY_URL_WITH_QUESTION_MARK_GET"
  local data = "GET /test.cgi?foo=bar?baz HTTP/1.1\r\n\r\n"

  local request = {
    should_keep_alive = true,
    message_complete_on_eof = false,
    method = http_native.GET,
    major = 1,
    minor = 1,
    schema = nil,
    auth = nil,
    host = nil,
    port = nil,
    path = "/test.cgi",
    query = "foo=bar?baz",
    hash = nil
  }

  local headers = {}

  local show = PRINT_RESULT
  local buffer_size = BUFFER_SIZE
  TEST_REQ(name, show, data, buffer_size, request, headers)
end

-- Some clients, especially after a POST in a keep-alive connection,
-- will send an extra CRLF before the next request
function PREFIX_NEWLINE_GET()
  local name = "PREFIX_NEWLINE_GET"
  local data = "\r\nGET /test HTTP/1.1\r\n\r\n"

  local request = {
    should_keep_alive = true,
    message_complete_on_eof = false,
    method = http_native.GET,
    major = 1,
    minor = 1,
    schema = nil,
    auth = nil,
    host = nil,
    port = nil,
    path = "/test",
    query = nil,
    hash = nil
  }

  local headers = {}

  local show = PRINT_RESULT
  local buffer_size = BUFFER_SIZE
  TEST_REQ(name, show, data, buffer_size, request, headers)
end

function UPGRADE_REQUEST()
  local name = "UPGRADE_REQUEST"
  local data = {
    "GET /demo HTTP/1.1\r\n",
    "Host: example.com\r\n",
    "Connection: Upgrade\r\n",
    "Sec-WebSocket-Key2: 12998 5 Y3 1  .P00\r\n",
    "Sec-WebSocket-Protocol: sample\r\n",
    "Upgrade: WebSocket\r\n",
    "Sec-WebSocket-Key1: 4 @1  46546xW%0l 1 5\r\n",
    "Origin: http://example.com\r\n",
    "\r\n",
    "Hot diggity dogg" 
  }

  local request = {
    should_keep_alive = true,
    message_complete_on_eof = false,
    method = http_native.GET,
    major = 1,
    minor = 1,
    schema = nil,
    auth = nil,
    host = nil,
    port = nil,
    path = "/demo",
    query = nil,
    hash = nil
  }

  local headers = {
    ["host"] = "example.com",
    ["connection"] = "Upgrade",
    ["sec-websocket-key2"] = "12998 5 Y3 1  .P00",
    ["sec-websocket-protocol"] = "sample",
    ["upgrade"] = "WebSocket",
    ["sec-websocket-key1"] = "4 @1  46546xW%0l 1 5",
    ["origin"] = "http://example.com"
  }

  local show = PRINT_RESULT
  local buffer_size = BUFFER_SIZE
  TEST_REQ(name, show, data, buffer_size, request, headers)
end

function CONNECT_REQUEST()
  local name = "CONNECT_REQUEST"
  local data = {
    "CONNECT 0-home0.netscape.com:443 HTTP/1.0\r\n",
    "User-agent: Mozilla/1.1N\r\n",
    "Proxy-authorization: basic aGVsbG86d29ybGQ=\r\n",
    "\r\n",
    "some data\r\n",
    "and yet even more data"
  }

  local request = {
    should_keep_alive = false,
    message_complete_on_eof = false,
    method = http_native.CONNECT,
    major = 1,
    minor = 0,
    schema = nil,
    auth = nil,
    host = "0-home0.netscape.com",
    port = "443",
    path = nil,
    query = nil,
    hash = nil
  }

  local headers = {
    ["user-agent"] = "Mozilla/1.1N",
    ["proxy-authorization"] = "basic aGVsbG86d29ybGQ=",
  }

  local show = PRINT_RESULT
  local buffer_size = BUFFER_SIZE
  TEST_REQ(name, show, data, buffer_size, request, headers)
end

function REPORT_REQ()
  local name = "REPORT_REQ"
  local data = {
    "REPORT /test HTTP/1.1\r\n",
    "\r\n"
  }

  local request = {
    should_keep_alive = true,
    message_complete_on_eof = false,
    method = http_native.REPORT,
    major = 1,
    minor = 1,
    schema = nil,
    auth = nil,
    host = nil,
    port = nil,
    path = "/test",
    query = nil,
    hash = nil
  }

  local headers = {}

  local show = PRINT_RESULT
  local buffer_size = BUFFER_SIZE
  TEST_REQ(name, show, data, buffer_size, request, headers)
end

function NO_HTTP_VERSION()
  local name = "NO_HTTP_VERSION"
  local data = {
    "GET /\r\n",
    "\r\n"
  }

  local request = {
    should_keep_alive = false,
    message_complete_on_eof = false,
    method = http_native.GET,
    major = 0,
    minor = 9,
    schema = nil,
    auth = nil,
    host = nil,
    port = nil,
    path = "/",
    query = nil,
    hash = nil
  }

  local headers = {}

  local show = PRINT_RESULT
  local buffer_size = BUFFER_SIZE
  TEST_REQ(name, show, data, buffer_size, request, headers)
end

function MSEARCH_REQ()
  local name = "MSEARCH_REQ"
  local data = {
    "M-SEARCH * HTTP/1.1\r\n",
    "HOST: 239.255.255.250:1900\r\n",
    "MAN: \"ssdp:discover\"\r\n",
    "ST: \"ssdp:all\"\r\n",
    "\r\n"
  }

  local request = {
    should_keep_alive = true,
    message_complete_on_eof = false,
    method = http_native.MSEARCH,
    major = 1,
    minor = 1,
    schema = nil,
    auth = nil,
    host = nil,
    port = nil,
    path = "*",
    query = nil,
    hash = nil
  }

  local headers = {
    ["host"] = "239.255.255.250:1900",
    ["man"] = "\"ssdp:discover\"",
    ["st"] = "\"ssdp:all\"",
  }

  local show = PRINT_RESULT
  local buffer_size = BUFFER_SIZE
  TEST_REQ(name, show, data, buffer_size, request, headers)
end

-- not support forever
function LINE_FOLDING_IN_HEADER()
  local name = "LINE_FOLDING_IN_HEADER"
  local data = {
    "GET / HTTP/1.1\r\n",
    "Line1:   abc\r\n",
    "\tdef\r\n",
    " ghi\r\n",
    "\t\tjkl\r\n",
    "  mno \r\n",
    "\t \tqrs\r\n",
    "Line2: \t line2\t\r\n",
    "Line3:\r\n",
    " line3\r\n",
    "Line4: \r\n",
    " \r\n",
    "Connection:\r\n",
    " close\r\n",
    "\r\n",
  }

  local request = {
    should_keep_alive = false,
    message_complete_on_eof = false,
    method = http_native.GET,
    major = 1,
    minor = 1,
    schema = nil,
    auth = nil,
    host = nil,
    port = nil,
    path = "/",
    query = nil,
    hash = nil
  }

  local headers = {
    ["line1"] = "abc\tdef ghi\t\tjkl  mno \t \tqrs",
    ["line2"] = "line2\t",
    ["line3"] = "line3",
    ["line4"] = "",
    ["connection"] = "close"
  }

  local show = PRINT_RESULT
  local buffer_size = BUFFER_SIZE
  TEST_REQ(name, show, data, buffer_size, request, headers)
end

-- host terminated by a query string
function QUERY_TERMINATED_HOST()
  local name = "QUERY_TERMINATED_HOST"
  local data = {
    "GET http://hypnotoad.org?hail=all HTTP/1.1\r\n",
    "\r\n"
  }

  local request = {
    should_keep_alive = true,
    message_complete_on_eof = false,
    method = http_native.GET,
    major = 1,
    minor = 1,
    schema = "http",
    auth = nil,
    host = "hypnotoad.org",
    port = nil,
    path = nil,
    query = "hail=all",
    hash = nil
  }

  local headers = {}

  local show = PRINT_RESULT
  local buffer_size = BUFFER_SIZE
  TEST_REQ(name, show, data, buffer_size, request, headers)
end

-- host:port terminated by a query string
function QUERY_TERMINATED_HOSTPORT()
  local name = "QUERY_TERMINATED_HOSTPORT"
  local data = {
    "GET http://hypnotoad.org:1234?hail=all HTTP/1.1\r\n",
    "\r\n"
  }

  local request = {
    should_keep_alive = true,
    message_complete_on_eof = false,
    method = http_native.GET,
    major = 1,
    minor = 1,
    schema = "http",
    auth = nil,
    host = "hypnotoad.org",
    port = "1234",
    path = nil,
    query = "hail=all",
    hash = nil
  }

  local headers = {}

  local show = PRINT_RESULT
  local buffer_size = BUFFER_SIZE
  TEST_REQ(name, show, data, buffer_size, request, headers)
end

-- host:port terminated by a space
function SPACE_TERMINATED_HOSTPORT()
  local name = "SPACE_TERMINATED_HOSTPORT"
  local data = {
    "GET http://hypnotoad.org:1234 HTTP/1.1\r\n",
    "\r\n"
  }

  local request = {
    should_keep_alive = true,
    message_complete_on_eof = false,
    method = http_native.GET,
    major = 1,
    minor = 1,
    schema = "http",
    auth = nil,
    host = "hypnotoad.org",
    port = "1234",
    path = nil,
    query = nil,
    hash = nil
  }

  local headers = {}

  local show = PRINT_RESULT
  local buffer_size = BUFFER_SIZE
  TEST_REQ(name, show, data, buffer_size, request, headers)
end

function PATCH_REQ()
  local name = "PATCH_REQ"
  local data = {
    "PATCH /file.txt HTTP/1.1\r\n",
    "Host: www.example.com\r\n",
    "Content-Type: application/example\r\n",
    "If-Match: \"e0023aa4e\"\r\n",
    "Content-Length: 10\r\n",
    "\r\n",
    "cccccccccc"
  }

  local request = {
    should_keep_alive = true,
    message_complete_on_eof = false,
    method = http_native.PATCH,
    major = 1,
    minor = 1,
    schema = nil,
    auth = nil,
    host = nil,
    port = nil,
    path = "/file.txt",
    query = nil,
    hash = nil
  }

  local headers = {
    ["host"] = "www.example.com",
    ["content-type"] = "application/example",
    ["if-match"] = "\"e0023aa4e\"",
    ["content-length"] = "10"  
  }

  local show = PRINT_RESULT
  local buffer_size = BUFFER_SIZE
  TEST_REQ(name, show, data, buffer_size, request, headers)
end

function CONNECT_CAPS_REQUEST()
  local name = "CONNECT_CAPS_REQUEST"
  local data = {
    "CONNECT HOME0.NETSCAPE.COM:443 HTTP/1.0\r\n",
    "User-agent: Mozilla/1.1N\r\n",
    "Proxy-authorization: basic aGVsbG86d29ybGQ=\r\n",
    "\r\n"
  }

  local request = {
    should_keep_alive = false,
    message_complete_on_eof = false,
    method = http_native.CONNECT,
    major = 1,
    minor = 0,
    schema = nil,
    auth = nil,
    host = "HOME0.NETSCAPE.COM",
    port = "443",
    path = nil,
    query = nil,
    hash = nil
  }

  local headers = {
    ["user-agent"] = "Mozilla/1.1N",
    ["proxy-authorization"] = "basic aGVsbG86d29ybGQ="
  }

  local show = PRINT_RESULT
  local buffer_size = BUFFER_SIZE
  TEST_REQ(name, show, data, buffer_size, request, headers)
end

-- not defined HTTP_PARSER_STRICT
function UTF8_PATH_REQ()
  local name = "UTF8_PATH_REQ"
  local data = {
    "GET /δ¶/δt/pope?q=1#narf HTTP/1.1\r\n",
    "Host: github.com\r\n",
    "\r\n"
  }

  local request = {
    should_keep_alive = true,
    message_complete_on_eof = false,
    method = http_native.GET,
    major = 1,
    minor = 1,
    schema = nil,
    auth = nil,
    host = nil,
    port = nil,
    path = "/δ¶/δt/pope",
    query = "q=1",
    hash = "narf"
  }

  local headers = {
    ["host"] = "github.com"
  }

  local show = PRINT_RESULT
  local buffer_size = BUFFER_SIZE
  TEST_REQ(name, show, data, buffer_size, request, headers)
end

-- not defined HTTP_PARSER_STRICT
function HOSTNAME_UNDERSCORE()
  local name = "HOSTNAME_UNDERSCORE"
  local data = {
    "CONNECT home_0.netscape.com:443 HTTP/1.0\r\n",
    "User-agent: Mozilla/1.1N\r\n",
    "Proxy-authorization: basic aGVsbG86d29ybGQ=\r\n",
    "\r\n"
  }

  local request = {
    should_keep_alive = false,
    message_complete_on_eof = false,
    method = http_native.GET,
    major = 1,
    minor = 1,
    schema = nil,
    auth = nil,
    host = nil,
    port = nil,
    path = nil,
    query = nil,
    hash = nil
  }

  local headers = {
    ["user-agent"] = "Mozilla/1.1N",
    ["proxy-authorization"] = "basic aGVsbG86d29ybGQ="
  }

  local show = PRINT_RESULT
  local buffer_size = BUFFER_SIZE
  TEST_REQ(name, show, data, buffer_size, request, headers)
end

-- see https://github.com/ry/http-parser/issues/47
-- eat CRLF between requests, no \"Connection: close\" header
function EAT_TRAILING_CRLF_NO_CONNECTION_CLOSE()
  local name = "EAT_TRAILING_CRLF_NO_CONNECTION_CLOSE"
  local data = {
    "POST / HTTP/1.1\r\n",
    "Host: www.example.com\r\n",
    "Content-Type: application/x-www-form-urlencoded\r\n",
    "Content-Length: 4\r\n",
    "\r\n",
    "q=42\r\n" -- note the trailing CRLF
  }

  local request = {
    should_keep_alive = true,
    message_complete_on_eof = false,
    method = http_native.POST,
    major = 1,
    minor = 1,
    schema = nil,
    auth = nil,
    host = nil,
    port = nil,
    path = "/",
    query = nil,
    hash = nil
  }

  local headers = {
    ["host"] = "www.example.com",
    ["content-type"] = "application/x-www-form-urlencoded",
    ["content-length"] = "4"
  }

  local show = PRINT_RESULT
  local buffer_size = BUFFER_SIZE
  TEST_REQ(name, show, data, buffer_size, request, headers)
end

-- see https://github.com/ry/http-parser/issues/47
-- eat CRLF between requests even if \"Connection: close\" is set
function EAT_TRAILING_CRLF_WITH_CONNECTION_CLOSE()
  local name = "EAT_TRAILING_CRLF_WITH_CONNECTION_CLOSE"
  local data = {
    "POST / HTTP/1.1\r\n",
    "Host: www.example.com\r\n",
    "Content-Type: application/x-www-form-urlencoded\r\n",
    "Content-Length: 4\r\n",
    "Connection: close\r\n",
    "\r\n",
    "q=42\r\n" -- note the trailing CRLF
  }

  local request = {
    should_keep_alive = false,
    message_complete_on_eof = false,
    method = http_native.POST,
    major = 1,
    minor = 1,
    schema = nil,
    auth = nil,
    host = nil,
    port = nil,
    path = "/",
    query = nil,
    hash = nil
  }

  local headers = {
    ["host"] = "www.example.com",
    ["content-type"] = "application/x-www-form-urlencoded",
    ["content-length"] = "4",
    ["connection"] = "close"
  }

  local show = PRINT_RESULT
  local buffer_size = BUFFER_SIZE
  TEST_REQ(name, show, data, buffer_size, request, headers)
end

function PURGE_REQ()
  local name = "PURGE_REQ"
  local data = {
    "PURGE /file.txt HTTP/1.1\r\n",
    "Host: www.example.com\r\n",
    "\r\n"
  }

  local request = {
    should_keep_alive = true,
    message_complete_on_eof = false,
    method = http_native.PURGE,
    major = 1,
    minor = 1,
    schema = nil,
    auth = nil,
    host = nil,
    port = nil,
    path = "/file.txt",
    query = nil,
    hash = nil
  }

  local headers = {
    ["host"] = "www.example.com"
  }

  local show = PRINT_RESULT
  local buffer_size = BUFFER_SIZE
  TEST_REQ(name, show, data, buffer_size, request, headers)
end

function SEARCH_REQ()
  local name = "SEARCH_REQ"
  local data = {
    "SEARCH / HTTP/1.1\r\n",
    "Host: www.example.com\r\n",
    "\r\n"
  }

  local request = {
    should_keep_alive = true,
    message_complete_on_eof = false,
    method = http_native.SEARCH,
    major = 1,
    minor = 1,
    schema = nil,
    auth = nil,
    host = nil,
    port = nil,
    path = "/",
    query = nil,
    hash = nil
  }

  local headers = {
    ["host"] = "www.example.com"
  }

  local show = PRINT_RESULT
  local buffer_size = BUFFER_SIZE
  TEST_REQ(name, show, data, buffer_size, request, headers)
end

-- host:port and basic_auth
function PROXY_WITH_BASIC_AUTH()
  local name = "PROXY_WITH_BASIC_AUTH"
  local data = {
    "GET http://a%12:b!&*$@hypnotoad.org:1234/toto HTTP/1.1\r\n",
    "\r\n"
  }

  local request = {
    should_keep_alive = true,
    message_complete_on_eof = false,
    method = http_native.GET,
    major = 1,
    minor = 1,
    schema = "http",
    auth = "a%12:b!&*$",
    host = "hypnotoad.org",
    port = "1234",
    path = "/toto",
    query = nil,
    hash = nil
  }

  local headers = {}

  local show = PRINT_RESULT
  local buffer_size = BUFFER_SIZE
  TEST_REQ(name, show, data, buffer_size, request, headers)
end

-- not support forever
-- line folding in header value
function LINE_FOLDING_IN_HEADER_WITH_LF()
  local name = "LINE_FOLDING_IN_HEADER_WITH_LF"
  local data = {
    "GET / HTTP/1.1\n",
    "Line1:   abc\n",
    "\tdef\n",
    " ghi\n",
    "\t\tjkl\n",
    "  mno \n",
    "\t \tqrs\n",
    "Line2: \t line2\t\n",
    "Line3:\n",
    " line3\n",
    "Line4: \n",
    " \n",
    "Connection:\n",
    " close\n",
    "\n"
  }

  local request = {
    should_keep_alive = false,
    message_complete_on_eof = false,
    method = http_native.GET,
    major = 1,
    minor = 1,
    schema = nil,
    auth = nil,
    host = nil,
    port = nil,
    path = "/",
    query = nil,
    hash = nil
  }

  local headers = {
    ["line1"] = "abc\tdef ghi\t\tjkl  mno \t \tqrs",
    ["line2"] = "line2\t",
    ["line3"] = "line3",
    ["line4"] = "",
    ["connection"] = "close"
  }

  local show = PRINT_RESULT
  local buffer_size = BUFFER_SIZE
  TEST_REQ(name, show, data, buffer_size, request, headers)
end

-- not support
-- multiple connection header values with folding
function CONNECTION_MULTI()
  local name = "CONNECTION_MULTI"
  local data = {
    "GET /demo HTTP/1.1\r\n",
    "Host: example.com\r\n",
    "Connection: Something,\r\n",
    " Upgrade, ,Keep-Alive\r\n",
    "Sec-WebSocket-Key2: 12998 5 Y3 1  .P00\r\n",
    "Sec-WebSocket-Protocol: sample\r\n",
    "Upgrade: WebSocket\r\n",
    "Sec-WebSocket-Key1: 4 @1  46546xW%0l 1 5\r\n",
    "Origin: http://example.com\r\n",
    "\r\n",
    "Hot diggity dogg"
  }

  local request = {
    should_keep_alive = true,
    message_complete_on_eof = false,
    method = http_native.GET,
    major = 1,
    minor = 1,
    schema = nil,
    auth = nil,
    host = nil,
    port = nil,
    path = "/demo",
    query = nil,
    hash = nil
  }

  local headers = {
    ["host"] = "example.com",
    ["connection"] = "Something, Upgrade, ,Keep-Alive",
    ["sec-websocket-key2"] = "12998 5 Y3 1  .P00",
    ["sec-websocket-protocol"] = "sample",
    ["upgrade"] = "WebSocket",
    ["sec-websocket-key1"] = "4 @1  46546xW%0l 1 5",
    ["origin"] = "http://example.com"
  }

  local show = PRINT_RESULT
  local buffer_size = BUFFER_SIZE
  TEST_REQ(name, show, data, buffer_size, request, headers)
end

-- multiple connection header values with folding and lws
function CONNECTION_MULTI_LWS()
  local name = "CONNECTION_MULTI_LWS"
  local data = {
    "GET /demo HTTP/1.1\r\n",
    "Connection: keep-alive, upgrade\r\n",
    "Upgrade: WebSocket\r\n",
    "\r\n",
    "Hot diggity dogg"
  }

  local request = {
    should_keep_alive = true,
    message_complete_on_eof = false,
    method = http_native.GET,
    major = 1,
    minor = 1,
    schema = nil,
    auth = nil,
    host = nil,
    port = nil,
    path = "/demo",
    query = nil,
    hash = nil
  }

  local headers = {
    ["connection"] = "keep-alive, upgrade",
    ["upgrade"] = "WebSocket"
  }

  local show = PRINT_RESULT
  local buffer_size = BUFFER_SIZE
  TEST_REQ(name, show, data, buffer_size, request, headers)
end

-- not support
-- multiple connection header values with folding and lws
function CONNECTION_MULTI_LWS_CRLF()
  local name = "CONNECTION_MULTI_LWS_CRLF"
  local data = {
    "GET /demo HTTP/1.1\r\n",
    "Connection: keep-alive, \r\n upgrade\r\n",
    "Upgrade: WebSocket\r\n",
    "\r\n",
    "Hot diggity dogg"
  }

  local request = {
    should_keep_alive = true,
    message_complete_on_eof = false,
    method = http_native.GET,
    major = 1,
    minor = 1,
    schema = nil,
    auth = nil,
    host = nil,
    port = nil,
    path = "/demo",
    query = nil,
    hash = nil
  }

  local headers = {
    ["connection"] = "keep-alive,  upgrade",
    ["upgrade"] = "WebSocket"
  }

  local show = PRINT_RESULT
  local buffer_size = BUFFER_SIZE
  TEST_REQ(name, show, data, buffer_size, request, headers)
end

function UPGRADE_POST_REQUEST()
  local name = "UPGRADE_POST_REQUEST"
  local data = {
    "POST /demo HTTP/1.1\r\n",
    "Host: example.com\r\n",
    "Connection: Upgrade\r\n",
    "Upgrade: HTTP/2.0\r\n",
    "Content-Length: 15\r\n",
    "\r\n",
    "sweet post body",
    "Hot diggity dogg"
  }

  local request = {
    should_keep_alive = true,
    message_complete_on_eof = false,
    method = http_native.POST,
    major = 1,
    minor = 1,
    schema = nil,
    auth = nil,
    host = nil,
    port = nil,
    path = "/demo",
    query = nil,
    hash = nil
  }

  local headers = {
    ["host"] = "example.com",
    ["connection"] = "Upgrade",
    ["upgrade"] = "HTTP/2.0",
    ["content-length"] = "15",
  }

  local show = PRINT_RESULT
  local buffer_size = BUFFER_SIZE
  TEST_REQ(name, show, data, buffer_size, request, headers)
end

function CONNECT_WITH_BODY_REQUEST()
  local name = "CONNECT_WITH_BODY_REQUEST"
  local data = {
    "CONNECT foo.bar.com:443 HTTP/1.0\r\n",
    "User-agent: Mozilla/1.1N\r\n",
    "Proxy-authorization: basic aGVsbG86d29ybGQ=\r\n",
    "Content-Length: 10\r\n",
    "\r\n",
    "blarfcicle"
  }

  local request = {
    should_keep_alive = false,
    message_complete_on_eof = false,
    method = http_native.CONNECT,
    major = 1,
    minor = 0,
    schema = nil,
    auth = nil,
    host = "foo.bar.com",
    port = "443",
    path = nil,
    query = nil,
    hash = nil
  }

  local headers = {
    ["user-agent"] = "Mozilla/1.1N",
    ["proxy-authorization"] = "basic aGVsbG86d29ybGQ=",
    ["content-length"] = "10"
  }

  local show = PRINT_RESULT
  local buffer_size = BUFFER_SIZE
  TEST_REQ(name, show, data, buffer_size, request, headers)
end

function REQUEST()
  print(color.blue('TESTING http request start'))

  CURL_GET()
  FIREFOX_GET()
  DUMBFUCK()
  FRAGMENT_IN_URI()
  GET_NO_HEADERS_NO_BODY()
  GET_ONE_HEADER_NO_BODY()
  GET_FUNKY_CONTENT_LENGTH()
  POST_IDENTITY_BODY_WORLD()
  POST_CHUNKED_ALL_YOUR_BASE()
  TWO_CHUNKS_MULT_ZERO_END()
  CHUNKED_W_TRAILING_HEADERS()
  CHUNKED_W_BULLSHIT_AFTER_LENGTH()
  WITH_QUOTES()
  APACHEBENCH_GET()
  QUERY_URL_WITH_QUESTION_MARK_GET()
  PREFIX_NEWLINE_GET()
  UPGRADE_REQUEST()
  CONNECT_REQUEST()
  REPORT_REQ()
  NO_HTTP_VERSION()
  MSEARCH_REQ()
  -- LINE_FOLDING_IN_HEADER()
  QUERY_TERMINATED_HOST()
  QUERY_TERMINATED_HOSTPORT()
  SPACE_TERMINATED_HOSTPORT()
  PATCH_REQ()
  CONNECT_CAPS_REQUEST()
  -- !HTTP_PARSER_STRICT
  -- UTF8_PATH_REQ()
  -- HOSTNAME_UNDERSCORE()
  EAT_TRAILING_CRLF_NO_CONNECTION_CLOSE()
  EAT_TRAILING_CRLF_WITH_CONNECTION_CLOSE()
  PURGE_REQ()
  SEARCH_REQ()
  PROXY_WITH_BASIC_AUTH()
  -- LINE_FOLDING_IN_HEADER_WITH_LF()
  -- CONNECTION_MULTI()
  CONNECTION_MULTI_LWS()
  -- CONNECTION_MULTI_LWS_CRLF()
  UPGRADE_POST_REQUEST()
  CONNECT_WITH_BODY_REQUEST()

  print(color.blue('TESTING http request ok'))
end

REQUEST()

function TEST_RES(name, show, data, buffer_size, res, vheaders, vcookies)
  if show then 
    print(color.blue('TESTING: ' .. name))
  end

  local path = DIR .. name .. '.txt'
  local fd = fs.open(path, 'w')
  fs.write(fd, data)
  fs.close(fd)

  fd = fs.open(path)
  local buffer = ReadBuffer.new(buffer_size)
  local size = fs.read(fd, buffer)

  local parser = http_native.new_parser()

  if res then
    local status, major, minor, err
    while true do
      status_code, major, minor, status, err = parser:parse_status_line(buffer)

      if err >= 0 or err == http_native.ERROR then break end

      if err == http_native.AGAIN then 
        size = fs.read(fd, buffer)
      end
    end

    if show then 
      print('parse_response error: ' .. err)
      print('status_code: ' .. status_code)
      if status ~= nil then
        print('status: ' .. status)
      end
      print('major: ' .. major)
      print('minor: ' .. minor)
    end

    assert(err == http_native.OK)
    assert(status_code == res.status_code)
    assert(status == res.status)
    assert(major == res.major)
    assert(minor == res.minor)
  end

  if vheaders or vcookies then
    local headers = {}
    local cookies = {}
    local err
    while true do
      err = parser:parse_headers(buffer, headers, cookies)
      if err ~= http_native.AGAIN then break end
      size = fs.read(fd, buffer)
    end

    if show then print('parse_headers error: ' .. err) end
    assert(err == http_native.OK)

    if vheaders then
      for k,v in pairs(headers) do
        if show then print(k .. ': ' .. v) end
        assert(headers[k] == vheaders[k])
      end
    end

    if vcookies then
      for i = 1, #vcookies do
        if show then print('cookie[' .. i .. ']: ' .. cookies[i]) end
        assert(cookies[i] == vcookies[i])
      end
    end
  end

  fs.close(fd)
end

function GOOGLE_301()
  local name = "GOOGLE_301"
  local data = {
    "HTTP/1.1 301 Moved Permanently\r\n",
    "Location: http://www.google.com/\r\n",
    "Content-Type: text/html; charset=UTF-8\r\n",
    "Date: Sun, 26 Apr 2009 11:11:49 GMT\r\n",
    "Expires: Tue, 26 May 2009 11:11:49 GMT\r\n",
    "X-$PrototypeBI-Version: 1.6.0.3\r\n", -- $ char in header field
    "Cache-Control: public, max-age=2592000\r\n",
    "Server: gws\r\n",
    "Content-Length:  219  \r\n",
    "\r\n",
    "<HTML><HEAD><meta http-equiv=\"content-type\" content=\"text/html;charset=utf-8\">\n",
    "<TITLE>301 Moved</TITLE></HEAD><BODY>\n",
    "<H1>301 Moved</H1>\n",
    "The document has moved\n",
    "<A HREF=\"http://www.google.com/\">here</A>.\r\n",
    "</BODY></HTML>\r\n"
  }

  local res = {
    should_keep_alive = true,
    message_complete_on_eof = false,
    status_code = 301,
    status = 'Moved Permanently',
    major = 1,
    minor = 1,
  }

  local headers = {
    ["location"] = "http://www.google.com/",
    ["content-type"] = "text/html; charset=UTF-8",
    ["date"] = "Sun, 26 Apr 2009 11:11:49 GMT",
    ["expires"] = "Tue, 26 May 2009 11:11:49 GMT",
    ["x-$prototypebi-version"] = "1.6.0.3",
    ["cache-control"] = "public, max-age=2592000",
    ["server"] = "gws",
    ["content-length"] = "219"
  }

  local show = PRINT_RESULT
  local buffer_size = BUFFER_SIZE
  TEST_RES(name, show, data, buffer_size, res, headers)
end

-- The client should wait for the server's EOF. That is, when content-length
-- is not specified, and "Connection: close", the end of body is specified by the EOF.
-- Compare with APACHEBENCH_GET
function NO_CONTENT_LENGTH_RESPONSE()
  local name = "NO_CONTENT_LENGTH_RESPONSE"
  local data = {
    "HTTP/1.1 200 OK\r\n",
    "Date: Tue, 04 Aug 2009 07:59:32 GMT\r\n",
    "Server: Apache\r\n",
    "X-Powered-By: Servlet/2.5 JSP/2.1\r\n",
    "Content-Type: text/xml; charset=utf-8\r\n",
    "Connection: close\r\n",
    "\r\n",
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n",
    "<SOAP-ENV:Envelope xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\">\n",
    "  <SOAP-ENV:Body>\n",
    "    <SOAP-ENV:Fault>\n",
    "       <faultcode>SOAP-ENV:Client</faultcode>\n",
    "       <faultstring>Client Error</faultstring>\n",
    "    </SOAP-ENV:Fault>\n",
    "  </SOAP-ENV:Body>\n",
    "</SOAP-ENV:Envelope>"
  }

  local res = {
    should_keep_alive = false,
    message_complete_on_eof = true,
    status_code = 200,
    status = 'OK',
    major = 1,
    minor = 1,
  }

  local headers = {
    ["date"] = "Tue, 04 Aug 2009 07:59:32 GMT",
    ["server"] = "Apache",
    ["x-powered-by"] = "Servlet/2.5 JSP/2.1",
    ["content-type"] = "text/xml; charset=utf-8",
    ["connection"] = "close"
  }

  local show = PRINT_RESULT
  local buffer_size = BUFFER_SIZE
  TEST_RES(name, show, data, buffer_size, res, headers)
end

function NO_HEADERS_NO_BODY_404()
  local name = "NO_HEADERS_NO_BODY_404"
  local data = "HTTP/1.1 404 Not Found\r\n\r\n"

  local res = {
    should_keep_alive = false,
    message_complete_on_eof = true,
    status_code = 404,
    status = 'Not Found',
    major = 1,
    minor = 1,
  }

  local headers = {}

  local show = PRINT_RESULT
  local buffer_size = BUFFER_SIZE
  TEST_RES(name, show, data, buffer_size, res, headers)
end

-- 301 no response phrase
function NO_REASON_PHRASE()
  local name = "NO_REASON_PHRASE"
  local data = "HTTP/1.1 301\r\n\r\n"

  local res = {
    should_keep_alive = false,
    message_complete_on_eof = true,
    status_code = 301,
    status = nil,
    major = 1,
    minor = 1,
  }

  local headers = {}

  local show = PRINT_RESULT
  local buffer_size = BUFFER_SIZE
  TEST_RES(name, show, data, buffer_size, res, headers)
end

-- 200 trailing space on chunked body
function TRAILING_SPACE_ON_CHUNKED_BODY()
  local name = "TRAILING_SPACE_ON_CHUNKED_BODY"
  local data = {
    "HTTP/1.1 200 OK\r\n",
    "Content-Type: text/plain\r\n",
    "Transfer-Encoding: chunked\r\n",
    "\r\n",
    "25  \r\n",
    "This is the data in the first chunk\r\n",
    "\r\n",
    "1C\r\n",
    "and this is the second one\r\n",
    "\r\n",
    "0  \r\n",
    "\r\n"
  }

  local res = {
    should_keep_alive = true,
    message_complete_on_eof = false,
    status_code = 200,
    status = 'OK',
    major = 1,
    minor = 1,
  }

  local headers = {
    ["content-type"] = "text/plain",
    ["transfer-encoding"] = "chunked"
  }

  local show = PRINT_RESULT
  local buffer_size = BUFFER_SIZE
  TEST_RES(name, show, data, buffer_size, res, headers)
end

function NO_CARRIAGE_RET()
  local name = "NO_CARRIAGE_RET"
  local data = {
    "HTTP/1.1 200 OK\n",
    "Content-Type: text/html; charset=utf-8\n",
    "Connection: close\n",
    "\n",
    "these headers are from http://news.ycombinator.com/"
  }

  local res = {
    should_keep_alive = false,
    message_complete_on_eof = true,
    status_code = 200,
    status = 'OK',
    major = 1,
    minor = 1,
  }

  local headers = {
    ["content-type"] = "text/html; charset=utf-8",
    ["connection"] = "close"
  }

  local show = PRINT_RESULT
  local buffer_size = BUFFER_SIZE
  TEST_RES(name, show, data, buffer_size, res, headers)
end

function PROXY_CONNECTION()
  local name = "PROXY_CONNECTION"
  local data = {
    "HTTP/1.1 200 OK\r\n",
    "Content-Type: text/html; charset=UTF-8\r\n",
    "Content-Length: 11\r\n",
    "Proxy-Connection: close\r\n",
    "Date: Thu, 31 Dec 2009 20:55:48 +0000\r\n",
    "\r\n",
    "hello world"
  }

  local res = {
    should_keep_alive = false,
    message_complete_on_eof = false,
    status_code = 200,
    status = 'OK',
    major = 1,
    minor = 1,
  }

  local headers = {
    ["content-type"] = "text/html; charset=UTF-8",
    ["content-length"] = "11",
    ["proxy-connection"] = "close",
    ["date"] = "Thu, 31 Dec 2009 20:55:48 +0000"
  }

  local show = PRINT_RESULT
  local buffer_size = BUFFER_SIZE
  TEST_RES(name, show, data, buffer_size, res, headers)
end

function UNDERSTORE_HEADER_KEY()
  local name = "UNDERSTORE_HEADER_KEY"
  local data = {
    "HTTP/1.1 200 OK\r\n",
    "Server: DCLK-AdSvr\r\n",
    "Content-Type: text/xml\r\n",
    "Content-Length: 0\r\n",
    "DCLK_imp: v7;x;114750856;0-0;0;17820020;0/0;21603567/21621457/1;;~okv=;dcmt=text/xml;;~cs=o\r\n\r\n"
  }

  local res = {
    should_keep_alive = true,
    message_complete_on_eof = false,
    status_code = 200,
    status = 'OK',
    major = 1,
    minor = 1,
  }

  local headers = {
    ["server"] = "DCLK-AdSvr",
    ["content-type"] = "text/xml",
    ["content-length"] = "0",
    ["dclk_imp"] = "v7;x;114750856;0-0;0;17820020;0/0;21603567/21621457/1;;~okv=;dcmt=text/xml;;~cs=o"
  }

  local show = PRINT_RESULT
  local buffer_size = BUFFER_SIZE
  TEST_RES(name, show, data, buffer_size, res, headers)
end

-- The client should not merge two headers fields when the first one doesn't have a value.
function BONJOUR_MADAME_FR()
  local name = "BONJOUR_MADAME_FR"
  local data = {
    "HTTP/1.0 301 Moved Permanently\r\n",
    "Date: Thu, 03 Jun 2010 09:56:32 GMT\r\n",
    "Server: Apache/2.2.3 (Red Hat)\r\n",
    "Cache-Control: public\r\n",
    "Pragma: \r\n",
    "Location: http://www.bonjourmadame.fr/\r\n",
    "Vary: Accept-Encoding\r\n",
    "Content-Length: 0\r\n",
    "Content-Type: text/html; charset=UTF-8\r\n",
    "Connection: keep-alive\r\n",
    "\r\n"
  }

  local res = {
    should_keep_alive = true,
    message_complete_on_eof = false,
    status_code = 301,
    status = 'Moved Permanently',
    major = 1,
    minor = 0,
  }

  local headers = {
    ["date"] = "Thu, 03 Jun 2010 09:56:32 GMT",
    ["server"] = "Apache/2.2.3 (Red Hat)",
    ["cache-control"] = "public",
    ["pragma"] = nil,
    ["location"] = "http://www.bonjourmadame.fr/",
    ["vary"] = "Accept-Encoding",
    ["content-length"] = "0",
    ["content-type"] = "text/html; charset=UTF-8",
    ["connection"] = "keep-alive"
  }

  local show = PRINT_RESULT
  local buffer_size = BUFFER_SIZE
  TEST_RES(name, show, data, buffer_size, res, headers)
end

function RES_FIELD_UNDERSCORE()
  local name = "RES_FIELD_UNDERSCORE"
  local data = {
    "HTTP/1.1 200 OK\r\n",
    "Date: Tue, 28 Sep 2010 01:14:13 GMT\r\n",
    "Server: Apache\r\n",
    "Cache-Control: no-cache, must-revalidate\r\n",
    "Expires: Mon, 26 Jul 1997 05:00:00 GMT\r\n",
    ".et-Cookie: PlaxoCS=1274804622353690521; path=/; domain=.plaxo.com\r\n",
    "Vary: Accept-Encoding\r\n",
    "_eep-Alive: timeout=45\r\n", -- semantic value ignored
    "_onnection: Keep-Alive\r\n", -- semantic value ignored
    "Transfer-Encoding: chunked\r\n",
    "Content-Type: text/html\r\n",
    "Connection: close\r\n",
    "\r\n",
    "0\r\n\r\n"
  }

  local res = {
    should_keep_alive = false,
    message_complete_on_eof = false,
    status_code = 200,
    status = 'OK',
    major = 1,
    minor = 1,
  }

  local headers = {
    ["date"] = "Tue, 28 Sep 2010 01:14:13 GMT",
    ["server"] = "Apache",
    ["cache-control"] = "no-cache, must-revalidate",
    ["expires"] = "Mon, 26 Jul 1997 05:00:00 GMT",
    [".et-cookie"] = "PlaxoCS=1274804622353690521; path=/; domain=.plaxo.com",
    ["vary"] = "Accept-Encoding",
    ["_eep-alive"] = "timeout=45",
    ["_onnection"] = "Keep-Alive",
    ["transfer-encoding"] = "chunked",
    ["content-type"] = "text/html",
    ["connection"] = "close"
  }

  local show = PRINT_RESULT
  local buffer_size = BUFFER_SIZE
  TEST_RES(name, show, data, buffer_size, res, headers)
end

function NON_ASCII_IN_STATUS_LINE()
  local name = "NON_ASCII_IN_STATUS_LINE"
  local data = {
    "HTTP/1.1 500 Oriëntatieprobleem\r\n",
    "Date: Fri, 5 Nov 2010 23:07:12 GMT+2\r\n",
    "Content-Length: 0\r\n",
    "Connection: close\r\n",
    "\r\n"
  }

  local res = {
    should_keep_alive = false,
    message_complete_on_eof = false,
    status_code = 500,
    status = 'Oriëntatieprobleem',
    major = 1,
    minor = 1,
  }

  local headers = {
    ["date"] = "Fri, 5 Nov 2010 23:07:12 GMT+2",
    ["content-length"] = "0",
    ["connection"] = "close"
  }

  local show = PRINT_RESULT
  local buffer_size = BUFFER_SIZE
  TEST_RES(name, show, data, buffer_size, res, headers)
end

function HTTP_VERSION_0_9()
  local name = "HTTP_VERSION_0_9"
  local data = {
    "HTTP/0.9 200 OK\r\n",
    "\r\n"
  }

  local res = {
    should_keep_alive = false,
    message_complete_on_eof = true,
    status_code = 200,
    status = 'OK',
    major = 0,
    minor = 9,
  }

  local headers = {}

  local show = PRINT_RESULT
  local buffer_size = BUFFER_SIZE
  TEST_RES(name, show, data, buffer_size, res, headers)
end

-- The client should wait for the server's EOF. That is, when neither
-- content-length nor transfer-encoding is specified, the end of body
-- is specified by the EOF.
-- neither content-length nor transfer-encoding response
function NO_CONTENT_LENGTH_NO_TRANSFER_ENCODING_RESPONSE()
  local name = "NO_CONTENT_LENGTH_NO_TRANSFER_ENCODING_RESPONSE"
  local data = {
    "HTTP/1.1 200 OK\r\n",
    "Content-Type: text/plain\r\n",
    "\r\n",
    "hello world"
  }

  local res = {
    should_keep_alive = false,
    message_complete_on_eof = true,
    status_code = 200,
    status = 'OK',
    major = 1,
    minor = 1,
  }

  local headers = {
    ["content-type"] = "text/plain"
  }

  local show = PRINT_RESULT
  local buffer_size = BUFFER_SIZE
  TEST_RES(name, show, data, buffer_size, res, headers)
end

-- HTTP/1.0 with keep-alive and EOF-terminated 200 status
function NO_BODY_HTTP10_KA_200()
  local name = "NO_BODY_HTTP10_KA_200"
  local data = {
    "HTTP/1.0 200 OK\r\n",
    "Connection: keep-alive\r\n",
    "\r\n"
  }

  local res = {
    should_keep_alive = false,
    message_complete_on_eof = true,
    status_code = 200,
    status = 'OK',
    major = 1,
    minor = 0,
  }

  local headers = {
    ["connection"] = "keep-alive"
  }

  local show = PRINT_RESULT
  local buffer_size = BUFFER_SIZE
  TEST_RES(name, show, data, buffer_size, res, headers)
end

-- HTTP/1.0 with keep-alive and a 204 status
function NO_BODY_HTTP10_KA_204()
  local name = "NO_BODY_HTTP10_KA_204"
  local data = {
    "HTTP/1.0 204 No content\r\n",
    "Connection: keep-alive\r\n",
    "\r\n"
  }

  local res = {
    should_keep_alive = true,
    message_complete_on_eof = false,
    status_code = 204,
    status = 'No content',
    major = 1,
    minor = 0,
  }

  local headers = {
    ["connection"] = "keep-alive"
  }

  local show = PRINT_RESULT
  local buffer_size = BUFFER_SIZE
  TEST_RES(name, show, data, buffer_size, res, headers)
end

-- HTTP/1.1 with an EOF-terminated 200 status
function NO_BODY_HTTP11_KA_200()
  local name = "NO_BODY_HTTP11_KA_200"
  local data = {
    "HTTP/1.1 200 OK\r\n",
    "\r\n"
  }

  local res = {
    should_keep_alive = false,
    message_complete_on_eof = true,
    status_code = 200,
    status = 'OK',
    major = 1,
    minor = 1,
  }

  local headers = {}

  local show = PRINT_RESULT
  local buffer_size = BUFFER_SIZE
  TEST_RES(name, show, data, buffer_size, res, headers)
end

-- HTTP/1.1 with a 204 status
function NO_BODY_HTTP11_KA_204()
  local name = "NO_BODY_HTTP11_KA_204"
  local data = {
    "HTTP/1.1 204 No content\r\n",
    "\r\n"
  }

  local res = {
    should_keep_alive = true,
    message_complete_on_eof = false,
    status_code = 204,
    status = 'No content',
    major = 1,
    minor = 1,
  }

  local headers = {}

  local show = PRINT_RESULT
  local buffer_size = BUFFER_SIZE
  TEST_RES(name, show, data, buffer_size, res, headers)
end

-- HTTP/1.1 with a 204 status and keep-alive disabled
function NO_BODY_HTTP11_NOKA_204()
  local name = "NO_BODY_HTTP11_NOKA_204"
  local data = {
    "HTTP/1.1 204 No content\r\n",
    "Connection: close\r\n",
    "\r\n"
  }

  local res = {
    should_keep_alive = false,
    message_complete_on_eof = false,
    status_code = 204,
    status = 'No content',
    major = 1,
    minor = 1,
  }

  local headers = {
    ["connection"] = "close"
  }

  local show = PRINT_RESULT
  local buffer_size = BUFFER_SIZE
  TEST_RES(name, show, data, buffer_size, res, headers)
end

-- HTTP/1.1 with chunked endocing and a 200 response
function NO_BODY_HTTP11_KA_CHUNKED_200()
  local name = "NO_BODY_HTTP11_KA_CHUNKED_200"
  local data = {
    "HTTP/1.1 200 OK\r\n",
    "Transfer-Encoding: chunked\r\n",
    "\r\n",
    "0\r\n",
    "\r\n"
  }

  local res = {
    should_keep_alive = true,
    message_complete_on_eof = false,
    status_code = 200,
    status = 'OK',
    major = 1,
    minor = 1,
  }

  local headers = {
    ["transfer-encoding"] = "chunked"
  }

  local show = PRINT_RESULT
  local buffer_size = BUFFER_SIZE
  TEST_RES(name, show, data, buffer_size, res, headers)
end

-- not support
-- Should handle spaces in header fields
function SPACE_IN_FIELD_RES()
  local name = "SPACE_IN_FIELD_RES"
  local data = {
    "HTTP/1.1 200 OK\r\n",
    "Server: Microsoft-IIS/6.0\r\n",
    "X-Powered-By: ASP.NET\r\n",
    "en-US Content-Type: text/xml\r\n", -- this is the problem
    "Content-Type: text/xml\r\n",
    "Content-Length: 16\r\n",
    "Date: Fri, 23 Jul 2010 18:45:38 GMT\r\n",
    "Connection: keep-alive\r\n",
    "\r\n",
    "<xml>hello</xml>" -- fake body
  }

  local res = {
    should_keep_alive = true,
    message_complete_on_eof = false,
    status_code = 200,
    status = 'OK',
    major = 1,
    minor = 1,
  }

  local headers = {
    ["server"] = "Microsoft-IIS/6.0",
    ["x-powered-by"] = "ASP.NET",
    ["en-us content-type"] = "text/xml",
    ["content-type"] = "text/xml",
    ["content-length"] = "16",
    ["date"] = "Fri, 23 Jul 2010 18:45:38 GMT",
    ["connection"] = "keep-alive"
  }

  local show = PRINT_RESULT
  local buffer_size = BUFFER_SIZE
  TEST_RES(name, show, data, buffer_size, res, headers)
end

function AMAZON_COM()
  local name = "AMAZON_COM"
  local data = {
    "HTTP/1.1 301 MovedPermanently\r\n",
    "Date: Wed, 15 May 2013 17:06:33 GMT\r\n",
    "Server: Server\r\n",
    "x-amz-id-1: 0GPHKXSJQ826RK7GZEB2\r\n",
    "p3p: policyref=\"http://www.amazon.com/w3c/p3p.xml\",CP=\"CAO DSP LAW CUR ADM IVAo IVDo CONo OTPo OUR DELi PUBi OTRi BUS PHY ONL UNI PUR FIN COM NAV INT DEM CNT STA HEA PRE LOC GOV OTC \"\r\n",
    "x-amz-id-2: STN69VZxIFSz9YJLbz1GDbxpbjG6Qjmmq5E3DxRhOUw+Et0p4hr7c/Q8qNcx4oAD\r\n",
    "Location: http://www.amazon.com/Dan-Brown/e/B000AP9DSU/ref=s9_pop_gw_al1?_encoding=UTF8&refinementId=618073011&pf_rd_m=ATVPDKIKX0DER&pf_rd_s=center-2&pf_rd_r=0SHYY5BZXN3KR20BNFAY&pf_rd_t=101&pf_rd_p=1263340922&pf_rd_i=507846\r\n",
    "Vary: Accept-Encoding,User-Agent\r\n",
    "Content-Type: text/html; charset=ISO-8859-1\r\n",
    "Transfer-Encoding: chunked\r\n",
    "\r\n",
    "1\r\n",
    "\n\r\n",
    "0\r\n",
    "\r\n"
  }

  local res = {
    should_keep_alive = true,
    message_complete_on_eof = false,
    status_code = 301,
    status = 'MovedPermanently',
    major = 1,
    minor = 1,
  }

  local headers = {
    ["date"] = "Wed, 15 May 2013 17:06:33 GMT",
    ["server"] = "Server",
    ["x-amz-id-1"] = "0GPHKXSJQ826RK7GZEB2",
    ["p3p"] = "policyref=\"http://www.amazon.com/w3c/p3p.xml\",CP=\"CAO DSP LAW CUR ADM IVAo IVDo CONo OTPo OUR DELi PUBi OTRi BUS PHY ONL UNI PUR FIN COM NAV INT DEM CNT STA HEA PRE LOC GOV OTC \"",
    ["x-amz-id-2"] = "STN69VZxIFSz9YJLbz1GDbxpbjG6Qjmmq5E3DxRhOUw+Et0p4hr7c/Q8qNcx4oAD",
    ["location"] = "http://www.amazon.com/Dan-Brown/e/B000AP9DSU/ref=s9_pop_gw_al1?_encoding=UTF8&refinementId=618073011&pf_rd_m=ATVPDKIKX0DER&pf_rd_s=center-2&pf_rd_r=0SHYY5BZXN3KR20BNFAY&pf_rd_t=101&pf_rd_p=1263340922&pf_rd_i=507846",
    ["vary"] = "Accept-Encoding,User-Agent",
    ["content-type"] = "text/html; charset=ISO-8859-1",
    ["transfer-encoding"] = "chunked"
  }

  local show = PRINT_RESULT
  local buffer_size = BUFFER_SIZE
  TEST_RES(name, show, data, buffer_size, res, headers)
end

function EMPTY_REASON_PHRASE_AFTER_SPACE()
  local name = "EMPTY_REASON_PHRASE_AFTER_SPACE"
  local data = {
    "HTTP/1.1 200 \r\n",
    "\r\n"
  }

  local res = {
    should_keep_alive = false,
    message_complete_on_eof = true,
    status_code = 200,
    status = nil,
    major = 1,
    minor = 1,
  }

  local headers = {}

  local show = PRINT_RESULT
  local buffer_size = BUFFER_SIZE
  TEST_RES(name, show, data, buffer_size, res, headers)
end

function RESPONSE()
  print(color.blue('TESTING http response start'))

  GOOGLE_301()
  NO_CONTENT_LENGTH_RESPONSE()
  NO_HEADERS_NO_BODY_404()
  NO_REASON_PHRASE()
  TRAILING_SPACE_ON_CHUNKED_BODY()
  NO_CARRIAGE_RET()
  PROXY_CONNECTION()
  UNDERSTORE_HEADER_KEY()
  BONJOUR_MADAME_FR()
  RES_FIELD_UNDERSCORE()
  NON_ASCII_IN_STATUS_LINE()
  HTTP_VERSION_0_9()
  NO_CONTENT_LENGTH_NO_TRANSFER_ENCODING_RESPONSE()
  NO_BODY_HTTP10_KA_200()
  NO_BODY_HTTP10_KA_204()
  NO_BODY_HTTP11_KA_200()
  NO_BODY_HTTP11_KA_204()
  NO_BODY_HTTP11_NOKA_204()
  NO_BODY_HTTP11_KA_CHUNKED_200()
  -- SPACE_IN_FIELD_RES()
  AMAZON_COM()
  EMPTY_REASON_PHRASE_AFTER_SPACE()

  print(color.blue('TESTING http response ok'))
end

RESPONSE()

local url_test = {
  {
    name = "proxy request",
    url = "http://hostname/",
    result = {
      schema = "http",
      auth = nil,
      host = "hostname",
      port = nil,
      path = "/",
      query = nil,
      hash = nil
    }
  },
  {
    name = "proxy request with port",
    url = "http://hostname:444/",
    result = {
      schema = "http",
      auth = nil,
      host = "hostname",
      port = "444",
      path = "/",
      query = nil,
      hash = nil
    }
  },
  {
    name = "CONNECT request",
    url = "hostname:443",
    is_connect = true,
    result = {
      schema = nil,
      auth = nil,
      host = "hostname",
      port = "443",
      path = nil,
      query = nil,
      hash = nil
    }
  },
  {
    name = "proxy ipv6 request",
    url = "http://[1:2::3:4]/",
    result = {
      schema = "http",
      auth = nil,
      host = "1:2::3:4",
      port = nil,
      path = "/",
      query = nil,
      hash = nil
    }
  },
  {
    name = "proxy ipv6 request with port",
    url = "http://[1:2::3:4]:67/",
    result = {
      schema = "http",
      auth = nil,
      host = "1:2::3:4",
      port = "67",
      path = "/",
      query = nil,
      hash = nil
    }
  },
  {
    name = "CONNECT ipv6 address",
    url = "[1:2::3:4]:443",
    is_connect = true,
    result = {
      schema = nil,
      auth = nil,
      host = "1:2::3:4",
      port = "443",
      path = nil,
      query = nil,
      hash = nil
    }
  },
  {
    name = "ipv4 in ipv6 address",
    url = "http://[2001:0000:0000:0000:0000:0000:1.9.1.1]/",
    result = {
      schema = "http",
      auth = nil,
      host = "2001:0000:0000:0000:0000:0000:1.9.1.1",
      port = nil,
      path = "/",
      query = nil,
      hash = nil
    }
  },
  {
    name = "extra ? in query string",
    url = "http://a.tbcdn.cn/p/fp/2010c/??fp-header-min.css,fp-base-min.css,fp-channel-min.css,fp-product-min.css,fp-mall-min.css,fp-category-min.css,fp-sub-min.css,fp-gdp4p-min.css,fp-css3-min.css,fp-misc-min.css?t=20101022.css",
    result = {
      schema = "http",
      auth = nil,
      host = "a.tbcdn.cn",
      port = nil,
      path = "/p/fp/2010c/",
      query = "?fp-header-min.css,fp-base-min.css,fp-channel-min.css,fp-product-min.css,fp-mall-min.css,fp-category-min.css,fp-sub-min.css,fp-gdp4p-min.css,fp-css3-min.css,fp-misc-min.css?t=20101022.css",
      hash = nil
    }
  },
  {
    name = "space URL encoded",
    url = "/toto.html?toto=a%20b",
    result = {
      schema = nil,
      auth = nil,
      host = nil,
      port = nil,
      path = "/toto.html",
      query = "toto=a%20b",
      hash = nil
    }
  },
  {
    name = "URL fragment",
    url = "/toto.html#titi",
    result = {
      schema = nil,
      auth = nil,
      host = nil,
      port = nil,
      path = "/toto.html",
      query = nil,
      hash = "titi"
    }
  },
  {
    name = "complex URL fragment",
    url = "http://www.webmasterworld.com/r.cgi?f=21&d=8405&url=http://www.example.com/index.html?foo=bar&hello=world#midpage",
    result = {
      schema = "http",
      auth = nil,
      host = "www.webmasterworld.com",
      port = nil,
      path = "/r.cgi",
      query = "f=21&d=8405&url=http://www.example.com/index.html?foo=bar&hello=world",
      hash = "midpage"
    }
  },
  {
    name = "complex URL from node js url parser doc",
    url = "http://host.com:8080/p/a/t/h?query=string#hash",
    result = {
      schema = "http",
      auth = nil,
      host = "host.com",
      port = "8080",
      path = "/p/a/t/h",
      query = "query=string",
      hash = "hash"
    }
  },
  {
    name = "complex URL with basic auth from node js url parser doc",
    url = "http://a:b@host.com:8080/p/a/t/h?query=string#hash",
    result = {
      schema = "http",
      auth = "a:b",
      host = "host.com",
      port = "8080",
      path = "/p/a/t/h",
      query = "query=string",
      hash = "hash"
    }
  },
  {
    name = "double @",
    url = "http://a:b@@hostname:443/",
    result = nil
  },
  {
    name = "proxy empty host",
    url = "http://:443/",
    result = nil
  },
  {
    name = "proxy empty port",
    url = "http://hostname:/",
    result = nil
  },
  {
    name = "CONNECT with basic auth",
    url = "a:b@hostname:443",
    is_connect = true,
    result = nil
  },
  {
    name = "CONNECT empty host",
    url = ":443",
    is_connect = true,
    result = nil
  },
  {
    name = "CONNECT empty port",
    url = "hostname:",
    is_connect = true,
    result = nil
  },
  {
    name = "CONNECT with extra bits",
    url = "hostname:443/",
    is_connect = true,
    result = nil
  },
  {
    name = "space in URL",
    url = "/foo bar/",
    result = nil
  },
  {
    name = "proxy basic auth with space url encoded",
    url = "http://a%20:b@host.com/",
    result = {
      schema = "http",
      auth = "a%20:b",
      host = "host.com",
      port = nil,
      path = "/",
      query = nil,
      hash = nil
    }
  },
  {
    name = "carriage return in URL",
    url = "/foo\rbar/",
    result = nil
  },
  {
    name = "proxy double : in URL",
    url = "http://hostname::443/",
    result = nil
  },
  {
    name = "proxy basic auth with double :",
    url = "http://a::b@host.com/",
    result = {
      schema = "http",
      auth = "a::b",
      host = "host.com",
      port = nil,
      path = "/",
      query = nil,
      hash = nil
    }
  },
  {
    name = "line feed in URL",
    url = "/foo\nbar/",
    result = nil
  },
  {
    name = "proxy empty basic auth",
    url = "http://@hostname/fo",
    result = {
      schema = "http",
      auth = nil,
      host = "hostname",
      port = nil,
      path = "/fo",
      query = nil,
      hash = nil
    }
  },
  {
    name = "proxy line feed in hostname",
    url = "http://host\name/fo",
    result = nil
  },
  {
    name = "proxy % in hostname",
    url = "http://host%name/fo",
    result = nil
  },
  {
    name = "proxy ; in hostname",
    url = "http://host;ame/fo",
    result = nil
  },
  {
    name = "proxy empty basic auth",
    url = "http://a!;-_!=+$@host.com/",
    result = {
      schema = "http",
      auth = "a!;-_!=+$",
      host = "host.com",
      port = nil,
      path = "/",
      query = nil,
      hash = nil
    }
  },
  {
    name = "proxy only empty basic auth",
    url = "http://@/fo",
    result = nil
  },
  {
    name = "proxy only basic auth",
    url = "http://toto@/fo",
    result = nil
  },
  {
    name = "proxy emtpy hostname",
    url = "http:///fo",
    result = nil
  },
  {
    name = "proxy = in URL",
    url = "http://host=ame/fo",
    result = nil
  },
  {
    name = "ipv6 address with Zone ID",
    url = "http://[fe80::a%25eth0]/",
    result = {
      schema = "http",
      auth = nil,
      host = "fe80::a%25eth0",
      port = nil,
      path = "/",
      query = nil,
      hash = nil
    }
  },
  {
    name = "ipv6 address with Zone ID, but '%' is not percent-encoded",
    url = "http://[fe80::a%eth0]/",
    result = {
      schema = "http",
      auth = nil,
      host = "fe80::a%eth0",
      port = nil,
      path = "/",
      query = nil,
      hash = nil
    }
  },
  {
    name = "ipv6 address ending with '%'",
    url = "http://[fe80::a%]/",
    result = nil
  },
  {
    name = "ipv6 address with Zone ID including bad character",
    url = "http://[fe80::a%$HOME]/",
    result = nil
  },
  {
    name = "just ipv6 Zone ID",
    url = "http://[%eth0]/",
    result = nil
  },
  {
    name = "tab in URL",
    url = "/foo\tbar/",
    result = nil
  },
  {
    name = "form feed in URL",
    url = "/foo\fbar/",
    result = nil
  }
--[[
  {
    name = "tab in URL",
    url = "/foo\tbar/",
    result = {
      schema = nil,
      auth = nil,
      host = nil,
      port = nil,
      path = "/foo\tbar/",
      query = nil,
      hash = nil
    }
  },
  {
    name = "form feed in URL",
    url = "/foo\fbar/",
    result = {
      schema = nil,
      auth = nil,
      host = nil,
      port = nil,
      path = "/foo\fbar/",
      query = nil,
      hash = nil
    }
  }
--]]
}

function URL()
  print(color.blue('TESTING: http_native.parse_url start'))

  for i = 1, #url_test do
    local case = url_test[i]
    
    if PRINT_RESULT then
      print(color.blue('CASE: ' .. case.name))
      print(color.green('URL: ' .. case.url))
    end

    local url = http_native.parse_url(case.url, case.is_connect)

    if case.result == nil then
      assert(url == nil)
    else 
      if PRINT_RESULT then 
        if url.schema then print('url.schema: ' .. url.schema) end
        if url.auth then print('url.auth: ' .. url.auth) end
        if url.host then print('url.host: ' .. url.host) end
        if url.port then print('url.port: ' .. url.port) end
        if url.path then print('url.path: ' .. url.path) end
        if url.query then print('url.query: ' .. url.query) end
        if url.hash then print('url.hash: ' .. url.hash) end
      end
    
      assert(url.schema == case.result.schema)
      assert(url.auth == case.result.auth)
      assert(url.host == case.result.host)
      assert(url.port == case.result.port)
      assert(url.path == case.result.path)
      assert(url.query == case.result.query)
      assert(url.hash == case.result.hash)
    end
  end

  print(color.blue('TESTING: http_native.parse_url ok'))
end

URL()

print(color.green('test_http_parser ok'))
