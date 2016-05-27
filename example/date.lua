local Date = require('date')

local co = coroutine.create(function()
print(Date.now())
print(Date.getUTCString())
print(Date.getLocalString())
print(Date.parseUTCString(Date.getUTCString()))
print(Date.parseLocalString(Date.getLocalString()))

sleep(10000)

print(Date.now())
print(Date.getUTCString())
print(Date.getLocalString())
print(Date.parseUTCString(Date.getUTCString()))
print(Date.parseLocalString(Date.getLocalString()))

end)

coroutine.resume(co)
