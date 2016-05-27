local process = require('process')

local options = {
  file = './chat_server.lua',
  forever = false
}

--for i = 1, 2 do
  --options.cpu = i - 1
  local pid = process.fork(options)
  print(pid)
--end
