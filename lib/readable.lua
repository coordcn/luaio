local ReadBuffer = require('read_buffer')
local Emitter = require('emitter')
local ERRNO = require('errno') 

local Readable = Emitter:extend()

-- @example: Readable.init(size)
-- @param: size {integer}
function Readable:init(size)
  local read_buffer, err = ReadBuffer:new(size)
  if err then
    self.errno = err
    self.init_fail = true
    self.closed = true
    return
  end

  self.read_buffer = read_buffer
  self.read_bytes = 0
  self.closed = false
end

function Readable:_read()
  error('Readable:_read() not implemented') 
end

-- @example: bytes = instance:bytesRead()
-- @return: bytes {integer}
function Readable:bytesRead()
  if self.init_fail then
    error('init fail: ' .. ERRNO.parse(self.errno))
  end

  return self.read_bytes
end

-- @example: instance:discard([n])
-- @param: n {integer} if n == nil read the rest data in the buffer
function Readable:discard(n)
  if self.init_fail then
    error('init fail: ' .. ERRNO.parse(self.errno))
  end

  if self.closed then
    error('closed, can not use')
  end

  if self.read_bytes == 0 then return end

  if n == nil then n = -1 end
  self.read_buffer:discard(n)
end

-- @example: data, err = instance:read([n])
-- @param: n {integer} if n == nil read the rest data in the buffer
-- @return: data {string}
-- @return: err {integer}
function Readable:read(n)
  if self.init_fail then
    error('init fail: ' .. ERRNO.parse(self.errno))
  end

  if self.closed then
    error('closed, can not use')
  end

  local ret
  if self.read_bytes == 0 then
    ret = self:_read()
    if ret < 0 then
      self.errno = ret
      return nil, ret
    end
    self.read_bytes = self.read_bytes + ret
  end
  
  if n == nil then n = -1 end
  local val, err = self.read_buffer:read(n)
  if err >= 0 then
    return val, err
  end

  while true do
    ret = self:_read()
    if ret < 0 then
      self._errno = ret
      return nil, ret
    end
    self.read_bytes = self.read_bytes + ret

    val, err = self.read_buffer:read(n)
    if err >= 0 then
      return val, err
    end
  end
end

-- @example: data, err = instance:readline()
-- @return: data {string}
-- @return: err {integer}
function Readable:readline()
  if self._init_fail then
    error('init fail: ' .. ERRNO.parse(self.errno))
  end

  if self._closed then
    error('closed, can not use')
  end

  local ret
  if self.read_bytes == 0 then
    ret = self:_read()
    if ret < 0 then
      self.errno = ret
      return nil, ret
    end
    self.read_bytes = self.read_bytes + ret
  end
  
  local val, err = self.read_buffer:readline()
  if err >= 0 then
    return val, err
  end

  if err == ERRNO.LUAIO_EXCEED_BUFFER_CAPACITY then
    self.errno = err
    return val, err
  end

  while true do
    ret = self:_read()
    if ret < 0 then 
      self.errno = ret
      return nil, ret
    end
    self.read_bytes = self.read_bytes + ret

    val, err = self.read_buffer:readline()
    if err >= 0 then
      return val, err
    end

    if err == ERRNO.LUAIO_EXCEED_BUFFER_CAPACITY then
      self.errno = err
      return val, err
    end
  end
end

function Readable:_readCommonType(name)
  if self.init_fail then
    error('init fail: ' .. ERRNO.parse(self.errno))
  end

  if self.closed then
    error('closed, can not use')
  end

  local func = ReadBuffer[name]

  local ret
  if self.read_bytes == 0 then
    ret = self:_read()
    if ret < 0 then
      self.errno = ret
      return nil, ret
    end
    self.read_bytes = self.read_bytes + ret
  end
  
  local val, err = func(self.read_buffer)
  if err >= 0 then
    return val, err
  end

  while true do
    ret = self:_read()
    if ret < 0 then
      self.errno = ret
      return nil, ret
    end
    self.read_bytes = self.read_bytes + ret

    val, err = func(self.read_buffer)
    if err >= 0 then
      return val, err
    end
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
