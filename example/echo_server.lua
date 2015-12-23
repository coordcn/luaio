local tcp = require('tcp')
local ERRNO = require('errno')

local num = 0
    local content = '中华人民共和国万岁\r\n'
  local msg = {
  'HTTP/1.1 200 OK\r\n',
  'Server: LINKS\r\n',
  'Connection: close\r\n',
  'Content-Type: text/html;charset=utf-8\r\n',
  'Date: Fri, 12 Jun 2015 03:53:40 GMT\r\n',
  'Content-Length: ' .. #content .. '\r\n',
  '\r\n',
  content
}

local function onconnect(socket)
  --num = num + 1
 -- socket.id = num
  --print('[' .. socket.id .. '] connected')

  --socket:on('close', function()
    --print('[' .. socket.id .. '] closed')
 -- end)

  local data, err
  while true do
    data, err = socket:read()
    if err < 0 then
      --print(ERRNO.parse(err))
      return
    end

    socket:write(msg)
    return
  end
end

local options = {
  reuseport = true
}

local server, err = tcp.createServer(8080, onconnect, options)
