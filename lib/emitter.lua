local Object = require('object')
local Set = require('set')

local Emitter = Object:extend()

function Emitter:on(name, callback)
  local events = rawget(self, '__EVENTS__')
  if not events then
    events = {}
    rawset(self, '__EVENTS__', events)
  end

  local handlers = rawget(events, name)
  if not handlers then
    handlers = Set:new()
    rawset(events, name, handlers)
  end

  handlers:add(callback)
end

function Emitter:off(name, callback)
  local events = rawget(self, "__EVENTS__")
  if not events then return end

  local handlers = rawget(events, name)
  if not handlers then return end

  if not callback then
    handlers:clear()
  else
    handlers:delete(callback)
  end
end

function Emitter:emit(name, ...)
  local events = rawget(self, "__EVENTS__")
  if not events then return false end

  local handlers = rawget(events, name)
  if not handlers or handlers.size == 0 then
    return false
  end

  local values = handlers.values
  local size = handlers.size
  for i = 1, size do
    values[i](...)
  end

--[[
  local args = {...}
  handlers:forEach(function(handle)
    handle(table.unpack(args))
  end)
--]]

  return true
end

return Emitter
