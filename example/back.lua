local tcp = require('tcp')
local ERRNO = require('errno')

local function onconnect(socket)
  local data, err
  while true do
    data, err = socket:read()
    if err < 0 then
      --print('back read error: ' .. ERRNO.parse(err))
      return
    end
    print(socket.server.connections)
    local mem = system.meminfo()
    print(mem.total)
    print(mem.free)
    bytes, err = socket:write(data)
    if err < 0 then
      --print('back write error: ' .. ERRNO.parse(err))
      return
    end
  end
end

local options = {
  reuseport = true
}

local server, err = tcp.createServer(8888, onconnect, options)
