local tcp = require('tcp')
local ERRNO = require('errno')
local send = require('./pool')

local function onconnect(socket)
  local data, err
  while true do
    data, err = socket:read()
    if err < 0 then
      print('front read error: ' .. ERRNO.parse(err))
      return
    end

    data, err = send(data)
    --print(err)
    if err < 0 then
      print('front send error: ' .. ERRNO.parse(err))
      return
    end

local msg = {
  'HTTP/1.1 200 OK\r\n',
  'Server: LINKS\r\n',
  'Content-Type: text/plain;charset=utf-8\r\n',
  --'Connection: close\r\n',
  'Content-Length: ' .. #data .. '\r\n',
  '\r\n',
  data
} 
    bytes, err = socket:write(msg)
    if err < 0 then
      print('front write error: ' .. ERRNO.parse(err))
      return
    end
  end
end

local options = {
  reuseport = true
}

local server, err = tcp.createServer(8080, onconnect, options)
