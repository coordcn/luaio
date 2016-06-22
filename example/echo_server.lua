local tcp = require('tcp')
local ERRNO = require('errno')

local num = 0
local content = '为生民立命，\r\n为天地立心，\r\n为往圣继绝学，\r\n为万世开太平！\r\n天行健，君子以自强不息；\r\n地势坤，君子以厚德载物。'
  local msg = {
  'HTTP/1.1 200 OK\r\n',
  'Server: LINKS\r\n',
  'Content-Type: text/plain;charset=utf-8\r\n',
 -- 'Connection: close\r\n',
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
 --
 --print(socket.server.connections)

  local data, err
  while true do
    data, err = socket:read()
    if err < 0 then
      --print(ERRNO.parse(err))
      return
    end

    bytes, err = socket:writeAsync(msg)
  end
end

local options = {
  reuseport = true
}

local server, err = tcp.createServer(8080, onconnect, options)
