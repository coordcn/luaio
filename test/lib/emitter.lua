local Object = require('object')
local Set = require('set')

local Emitter = Object:extend()

function Emitter:on(name, callback)
  local events = rawget(self, '_EVENTS_')
  if not events then
    events = {}
    rawset(self, '_EVENTS_', events)
  end

  local handlers = rawget(events, name)
  if not handlers then
    handlers = Set:new()
    rawset(events, name, handlers)
  end

  handlers:add(callback)
end

function Emitter:off(name, callback)
  local events = rawget(self, "_EVENTS_")
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
  local events = rawget(self, "_EVENTS_")
  if not events then return false end

  local handlers = rawget(events, name)
  if not handlers or handlers.size == 0 then
    return false
  end

  handlers:forEach(function(handle)
    handle(self, ...)
  end)

  return true
end

return Emitter
