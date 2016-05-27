local tcp = require('tcp')
local ERRNO = require('errno')
local fs = require('fs')

local request = {
'GET / HTTP/1.1\r\n',
'Host: www.lagou.com\r\n',
'User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:31.0) Gecko/20100101 Firefox/31.0\r\n',
'Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n',
'Accept-Language: zh-cn,zh;q=0.8,en-us;q=0.5,en;q=0.3\r\n',
'Accept-Encoding: gzip, deflate\r\n',
'Connection: keep-alive\r\n\r\n'
}

print(process.execpath)
--co = coroutine.create(function()
  local socket, err = tcp.connect(80, 'www.lagou.com', {timeout = 30000})
  print(ERRNO.parse(err))

  local stream, err = fs.createWriteStream('t.txt')

    bytes, err = socket:write(request)
    print(bytes)
    if err < 0 then
      print('write: ' .. ERRNO.parse(err))
      return
    end

  while true do
    local data
    data, err = socket:read()
    if err < 0 then
      print('read: ' .. ERRNO.parse(err))
      break
    end
    stream:write(data)
  end

  stream:close()
--end)
--coroutine.resume(co)
