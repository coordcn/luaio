local tcp = require('tcp')
local Set = require('set')
local ERRNO = require('errno')

local room = Set:new()
local num = 0

local function onconnect(socket)
  num = num + 1
  socket.id = num
  print('[' .. socket.id .. '] connected')
  room:add(socket)

  socket:on('close', function()
    print('[' .. socket.id .. '] closed')
    room:delete(socket)
  end)

  local data, err, msg
  while true do
    data, err = socket:read()
    if err < 0 then
      print(ERRNO.parse(err))
      return
    end
    
    local msg = '[' .. socket.id .. '] say: ' .. data
    room:forEach(function(sock)
      err = sock:write(msg)
    end)
  end
end

local options = {
  reuseport = true
}

local server, err = tcp.createServer(9527, onconnect, options)
