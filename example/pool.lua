local tcp = require('tcp')
local ERRNO = require('errno')

local pool = {}
local size = 0
local size_max = 256

local host = '127.0.0.1'
local port = 8888

local function send(data)
  local socket, err
  if size > 0 then
    socket = table.remove(pool)
    size = size - 1
    if size > 400 then print(size) end
  else
    socket, err = tcp.connect(port)
    if err < 0 then
      print('tcp.connect error: ' .. ERRNO.parse(err))
      return nil, err
    end
  end

  local bytes
  bytes, err = socket:write(data)
  if err < 0 then
    print('pool write error: ' .. ERRNO.parse(err))
    socket:close()
    return nil, err
  end

  local data
  data, err = socket:read()
  if err < 0 then
    print('pool read error: ' .. ERRNO.parse(err))
    socket:close()
    return nil, err
  end

  if size < size_max then
    table.insert(pool, socket)
    size = size + 1
  else
    socket:close()
  end

  return data, err
end

return send
