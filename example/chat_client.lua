local tcp = require('tcp')
local ERRNO = require('errno')

co = coroutine.create(function()
  local socket, err = tcp.connect(9527, '127.0.0.1')
  print(ERRNO.parse(err))

  while true do
    err = socket:write('hello world')
    if err < 0 then
      print(ERRNO.parse(err))
      return
    end

    local data
    data, err = socket:read()
    if err < 0 then
      print(ERRNO.parse(err))
      return
    end

    print(data)
  end
end)
coroutine.resume(co)
