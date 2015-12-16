local tcp = require('tcp')
local ERRNO = require('errno')

local room = {}
local num = 0

local function onconnect(socket)
  num = num + 1
  local role = {
    id = num,
    socket = socket
  }
  table.insert(room, role)

  local data, err, msg
  while true do
    data, err = socket:read()
    if err then
      -- TODO delete the socket from room
      print(ERRNO.parse(err))
      return
    end
    
    msg = '[' .. role.id .. ']' .. data

    for i = 1, #room do
      if room[i].socket then
        err = room[i].socket:write(data)
        print(ERRNO.parse(err))
      end
    end
  end
end

local options = {
  reuseport = true
}

local server, err = tcp.createServer(9527, onconnect, options)
