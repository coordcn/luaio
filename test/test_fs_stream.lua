local color = require('color')
local fs = require('fs')
local WriteBuffer = require('write_buffer')
local ERRNO = require('errno')

local uint8 = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08}
local uint16 = {0x0102, 0x0304, 0x0506, 0x0708}
local uint32 = {0x01020304, 0x05060708}
local uint64 = 0x0102030405060708
local int8 = {-0x01, -0x02, -0x03, -0x04, -0x05, -0x06, -0x07, -0x08}
local int16 = {-0x0102, -0x0304, -0x0506, -0x0708}
local int32 = {-0x01020304, -0x05060708}
local int64 = -0x0102030405060708
local float = 2.0
local double = 4.0
local str1 = '中华人民共和国万岁！'
local str2 = '世界人民大团结万岁！'

function main()
  local write_buffer = WriteBuffer.new(512)
  assert(write_buffer, color.red('test_fs_stream [WriteBuffer.new(size)] error'))
  assert(write_buffer:capacity() == 512, color.red('test_fs_stream [WriteBuffer:capacity()] error'))

  local ret = write_buffer:write(str1 .. str2)
  assert(ret == #str1 + #str2, color.red('test_fs_stream [WriteBuffer:write(string)] error'))

  write_buffer:discard(#str1)

  for i = 1, #uint8 do
    ret = write_buffer:write_uint8(uint8[i])
    assert(ret == 1, color.red('test_fs_stream [WriteBuffer:write_uint8(' .. uint8[i] .. ')] error'))
  end

  for i = 1, #uint16 do
    ret = write_buffer:write_uint16_le(uint16[i])
    assert(ret == 2, color.red('test_fs_stream [WriteBuffer:write_uint16_le(' .. uint16[i] .. ')] error'))
  end

  for i = 1, #uint32 do
    ret = write_buffer:write_uint32_le(uint32[i])
    assert(ret == 4, color.red('test_fs_stream [WriteBuffer:write_uint32_le(' .. uint32[i] .. ')] error'))
  end

  ret = write_buffer:write_uint64_le(uint64)
  assert(ret == 8, color.red('test_fs_stream [WriteBuffer:write_uint64_le(' .. uint64 .. ')] error'))

  for i = 1, #int8 do
    ret = write_buffer:write_int8(int8[i])
    assert(ret == 1, color.red('test_fs_stream [WriteBuffer:write_int8(' .. int8[i] .. ')] error'))
  end

  for i = 1, #int16 do
    ret = write_buffer:write_int16_le(int16[i])
    assert(ret == 2, color.red('test_fs_stream [WriteBuffer:write_int16_le(' .. int16[i] .. ')] error'))
  end

  for i = 1, #int32 do
    ret = write_buffer:write_int32_le(int32[i])
    assert(ret == 4, color.red('test_fs_stream [WriteBuffer:write_int32_le(' .. int32[i] .. ')] error'))
  end

  ret = write_buffer:write_int64_le(int64)
  assert(ret == 8, color.red('test_fs_stream [WriteBuffer:write_int64_le(' .. int64 .. ')] error'))

  ret = write_buffer:write_float_le(float)
  assert(ret == 4, color.red('test_fs_stream [WriteBuffer:write_float_le(' .. float .. ')] error'))

  ret = write_buffer:write_double_le(double)
  assert(ret == 8, color.red('test_fs_stream [WriteBuffer:write_double_le(' .. double .. ')] error'))

  local write_stream = fs.createWriteStream('./test.txt')

  ret = write_stream:write(write_buffer)

  print(color.green('test_fs_stream ok'))
end

return main
