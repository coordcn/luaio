local parallel = require('parallel')
local fs = require('fs')
local ERRNO = require('errno')

local args = {
  't1.txt',
  't2.txt',
  't3.txt',
  't4.txt',
  't5.txt'
}

local co = coroutine.create(function()
  local res = parallel.map(args, fs.readFile, 2)

  for i = 1, #res do
    local item = res[i]
    print('error: ' .. item[2])
    print('value:' .. item[1])
  end
end)

coroutine.resume(co)
