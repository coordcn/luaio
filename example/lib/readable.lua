local ReadBuffer = require('read_buffer')
local Emitter = require('emitter')
local ERRNO = require('errno') 

local Readable = Emitter:extend()

-- @example: local err = Readable.init(self, size)
-- @param: self {table} child instance
-- @param: size {integer}
-- @return: err {integer}
function Readable:init(size)
  local read_buffer = ReadBuffer.new(size)
  if not read_buffer then return ERRNO.UV_ENOMEM end

  self.read_buffer = read_buffer
  self.read_bytes = 0
  self.closed = false
  return 0
end

function Readable:_read()
  error('Readable:_read() not implemented') 
end

-- @example: local bytes = instance:bytesRead()
-- @return: bytes {integer}
function Readable:bytesRead()
  return self.read_bytes
end

-- @example: instance:discard([n])
-- @param: n {integer} if n == nil read the rest data in the buffer
function Readable:discard(n)
  if self.closed then error('closed, unavaliable') end
  if self.read_bytes == 0 then return end
  if n == nil then n = -1 end
  self.read_buffer:discard(n)
end

-- @example: local data, err = instance:read([n])
-- @param: n {integer} if n == nil read the rest data in the buffer
-- @return: data {string}
-- @return: err {integer}
function Readable:read(n)
  if self.closed then error('closed, unavaliable') end

  local ret
  if self.read_bytes == 0 then
    ret = self:_read()
    if ret < 0 then return nil, ret end
    self.read_bytes = self.read_bytes + ret
  end
  
  if n == nil then n = -1 end
  local val, err = self.read_buffer:read(n)
  if err > 0 then return val, err end

  while true do
    ret = self:_read()
    if ret < 0 then return nil, ret end
    self.read_bytes = self.read_bytes + ret

    val, err = self.read_buffer:read(n)
    if err >= 0 then return val, err end
  end
end

-- @example: local data, err = instance:readline()
-- @return: data {string}
-- @return: err {integer}
function Readable:readline()
  if self.closed then error('closed, unavaliable') end

  local ret
  if self.read_bytes == 0 then
    ret = self:_read()
    if ret < 0 then return nil, ret end
    self.read_bytes = self.read_bytes + ret
  end
  
  local val, err = self.read_buffer:readline()
  if err > 0 then return val, err end
  if err == ERRNO.LUAIO_EXCEED_BUFFER_CAPACITY then
    return val, err
  end

  while true do
    ret = self:_read()
    if ret < 0 then return nil, ret end
    self.read_bytes = self.read_bytes + ret

    val, err = self.read_buffer:readline()
    if err >= 0 then return val, err end
    if err == ERRNO.LUAIO_EXCEED_BUFFER_CAPACITY then
      return val, err
    end
  end
end

-- @example: local ret, err = instance:_readCommonType()
-- @return: ret {integer|number}
-- @return: err {integer}
function Readable:_readCommonType(name)
  if self.closed then error('closed, unavaliable') end

  local ret
  local func = ReadBuffer[name]
  if self.read_bytes == 0 then
    ret = self:_read()
    if ret < 0 then return nil, ret end
    self.read_bytes = self.read_bytes + ret
  end
  
  local val, err = func(self.read_buffer)
  if err > 0 then return val, err end

  while true do
    ret = self:_read()
    if ret < 0 then return nil, ret end
    self.read_bytes = self.read_bytes + ret

    val, err = func(self.read_buffer)
    if err > 0 then return val, err end
  end
end

function Readable:readUInt8()
  self._readCommonType('read_uint8')
end

function Readable:readInt8()
  self._readCommonType('read_int8')
end

function Readable:readUInt16LE()
  self._readCommonType('read_uint16_le')
end

function Readable:readUInt16BE()
  self._readCommonType('read_uint16_be')
end

function Readable:readUInt32LE()
  self._readCommonType('read_uint32_le')
end

function Readable:readUInt32BE()
  self._readCommonType('read_uint32_be')
end

function Readable:readUInt64LE()
  self._readCommonType('read_uint64_le')
end

function Readable:readInt64BE()
  self._readCommonType('read_int64_be')
end

function Readable:readInt16LE()
  self._readCommonType('read_int16_le')
end

function Readable:readInt16BE()
  self._readCommonType('read_int16_be')
end

function Readable:readInt32LE()
  self._readCommonType('read_int32_le')
end

function Readable:readInt32BE()
  self._readCommonType('read_int32_be')
end

function Readable:readInt64LE()
  self._readCommonType('read_int64_le')
end

function Readable:readInt64BE()
  self._readCommonType('read_int64_be')
end

function Readable:readFloatLE()
  self._readCommonType('read_float_le')
end

function Readable:readFloatBE()
  self._readCommonType('read_float_be')
end

function Readable:readDoubleLE()
  self._readCommonType('read_double_le')
end

function Readable:readDoubleBE()
  self._readCommonType('read_double_be')
end

return Readable
