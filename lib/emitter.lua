local Object = require('object')

local Emitter = Object:extend()

function Emitter:on(name, callback)
  local events = rawget(self, "events")
  if not events then
    events = {}
    rawset(self, "events", events)
  end

  local events_type = rawget(events, name)
  if not events_type then
    events_type = {}
    rawset(events, name, events_type)
  end

  table.insert(events_type, function(...) 
    return callback(self, ...)
  end)
end

function Emitter:emit(name, ...)
  local events = rawget(self, "events")
  if not events then 
    return 
  end

  local events_type = rawget(events, name)
  if not events_type then
    return
  end

  for i = 1, #events_type do
    events_type[i](...)
  end
end

return Emitter
