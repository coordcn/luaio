local fs = require('fs')
local ERRNO = require('errno')
local co1 = coroutine.create(function()
local count = 0;
local current = coroutine.running()

local co11 = coroutine.create(function()
  print('co11 start')
  local rstream, err, data
  local lines = {}
  rstream, err = fs.createReadStream('t1.txt')
  if err < 0 then print(ERRNO.parse(err)) end

  while true do
    data, err = rstream:readline()
    if err < 0 then
      if err == ERRNO.UV_EOF then
        break
      end

      print(ERRNO.parse(err))
      break
    end
    print('co11: ' .. data)
    table.insert(lines, data)
  end

  for i = 1, #lines do
    print('c11: ' .. lines[i])
  end

  count = count + 1
  print(count)
  if count == 2 then
    coroutine.resume(current)
  end
end)

local co12 = coroutine.create(function()
  print('co12 start')
  local rstream, err, data
  local lines = {}
  rstream, err = fs.createReadStream('t1.txt')
  if err < 0 then print(ERRNO.parse(err)) end

  while true do
    data, err = rstream:readline()
    if err < 0 then
      if err == ERRNO.UV_EOF then
        break
      end

      print(ERRNO.parse(err))
      break
    end

    print('co12: ' .. data)
    table.insert(lines, data)
  end

  for i = 1, #lines do
    print('c12: ' .. lines[i])
  end

  count = count + 1
  print(count)
  if count == 2 then
    coroutine.resume(current)
  end
end)

coroutine.resume(co11)
print('start co11')
coroutine.resume(co12)
coroutine.yield()
print('start co12')
end)

local co2 = coroutine.create(function()
local count = 0;
local current = coroutine.running()

local co21 = coroutine.create(function()
  print('co21 start')
  local rstream, err, data
  local lines = {}
  rstream, err = fs.createReadStream('t1.txt')
  if err < 0 then print(ERRNO.parse(err)) end

  while true do
    data, err = rstream:readline()
    if err < 0 then
      if err == ERRNO.UV_EOF then
        break
      end

      print(ERRNO.parse(err))
      break
    end
    print('co21: ' .. data)
    table.insert(lines, data)
  end

  for i = 1, #lines do
    print('c21: ' .. lines[i])
  end

  count = count + 1
  print(count)
  if count == 2 then
    coroutine.resume(current)
  end
end)

local co22 = coroutine.create(function()
  print('co22 start')
  local rstream, err, data
  local lines = {}
  rstream, err = fs.createReadStream('t1.txt')
  if err < 0 then print(ERRNO.parse(err)) end

  while true do
    data, err = rstream:readline()
    if err < 0 then
      if err == ERRNO.UV_EOF then
        break
      end

      print(ERRNO.parse(err))
      break
    end

    print('co22: ' .. data)
    table.insert(lines, data)
  end

  for i = 1, #lines do
    print('c22: ' .. lines[i])
  end

  count = count + 1
  print(count)
  if count == 2 then
    coroutine.resume(current)
  end
end)

coroutine.resume(co21)
print('start co21')
coroutine.resume(co22)
coroutine.yield()
print('start co22')
end)

coroutine.resume(co1)
print('start co1')
coroutine.resume(co2)
print('start co2')
