local process = require('process')

local options = {
  forever = false
}

for i = 1, 3 do
  --options.cpu = i - 1
  local pid = process.fork('./echo_server.lua', options)
  print(pid)
end
