Object = require('object')

local Set = Object:extend()

function Set:init()
  self.size = 0
  self.keys = {}
  self.values = {}
  return 0
end

function Set:add(value)
  local keys = self.keys
  if not keys[value] then
    table.insert(self.values, value)
    self.size = self.size + 1
    keys[value] = self.size
  end
end

function Set:delete(value)
  local keys = self.keys
  local values = self.values
  local index = keys[value]
  if index then
    keys[value] = nil
    local top = table.remove(values)
    if top ~= value then
      keys[top] = index
      values[index] = top
    end

    self.size = self.size - 1
  end
end

function Set:clear()
  if self.size ~= 0 then
    self.values = {}
    self.keys = {}
    self.size = 0
  end
end

function Set:has(value)
  if self.keys[value] then 
    return true
  end

  return false
end

function Set:forEach(callback)
  for i = 1, self.size do
    callback(self.values[i])
  end
end

return Set
