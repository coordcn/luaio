local fs = require('fs')
local ERRNO = require('errno')

local co = coroutine.create(function()
  local files, err = fs.readdir('.')
  if err < 0 then
    print(ERRNO.parse(err))
  end

  for i = 1, #files do
    local match = string.match(files[i], "^test%_(.*).lua$")
    if match then
      local path = "./test_" .. match
      print('testing: ' .. path)
      require('test_' .. match)()
    end
  end
end)

coroutine.resume(co)  
