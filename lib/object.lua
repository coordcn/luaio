-- @overview: frome luvit deps/core.lua

local ERRNO = require('errno')

local Object = {
  errno = 0,
  init_fail = false,
  meta = {
    __index = Object
  }
}
 
-- @overview: create a new instance of this object
function Object:create()
  local meta = rawget(self, 'meta')
  if not meta then error('cannot inherit form instance object') end
  return setmetatable({}, meta)
end

-- @overview: creates a new instance and calls obj:init(...) if it exists.
-- @example:
--    local Rectangle = Object:extend()
--    function Rectangle:init(w, h)
--      self.w = w
--      self.h = h
--    end
--
--    function Rectangle:getArea()
--      return self.w * self.h
--    end
--
--    local rect = Rectangle:new(3, 4)
--    print(rect:getArea())
function Object:new(...)
  local object = self:create()
  if type(object.init) == "function" then
    object:init(...)
  end
  return object
end

-- @overview: creates a new sub-class.
-- @example:
--    local Square = Rectangle:extend()
--    function Square:init(w)
--      self.w = w
--      self.h = h
--    end
function Object:extend()
  local object = self:create()
  local meta = {}
  -- move the meta methods defined in our ancestors meta into our own
  -- to preserve expected behavior in children (like __tostring, __add, etc)
  for k, v in pairs(self.meta) do
    meta[k] = v
  end
  meta.__index = object
  object.meta = meta
  return object
end

function Object:errno()
  return self.errno
end

function Object:errstr()
  return ERRNO.parse(self.errno)
end

return Object
