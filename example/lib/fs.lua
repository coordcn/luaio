local fs_native = require('fs_native')
local ReadBuffer = require('read_buffer')
local Object = require('object')
local Readable = require('readable')
local ERRNO = require('errno')

local fs = {}

-- @example: local ret = fs.access(path[, mode])
-- @param: path {string}
-- @param: mode {string}
--    F_OK = 0 test for existence
--    R_OK = 4 test for read permission
--    W_OK = 2 test for write permission
--    X_OK = 1 test for execute permission
--    if mode == nil test F_OK
--    if mode == 'r' test F_OK, R_OK
--    if mode == 'w' test F_OK, W_OK
--    if mode == 'x' test F_OK, X_OK
--    if mode == 'rw' test F_OK, R_OK, W_OK
--    if mode == 'rx' test F_OK, R_OK, X_OK
--    if mode == 'wx' test F_OK, W_OK, X_OK
--    if mode == 'rwx' test F_OK, R_OK, W_OK, X_OK
-- @return: ret {integer} if ret < 0 ret is errno
function fs.access(path, mode)
  if mode == nil then mode = 0 end
  return fs_native.access(path, mode)
end

-- @execute: local err = fs.close(fd)
-- @param: fd {integer}
-- @return: err {integer}
function fs.close(fd)
  return fs_native.close(fd)
end

-- @example: local ret = fs.open(path[, flag, mode])
-- @param: path {string}
-- @param: flag {string}
-- @param: mode {integer}
-- @return: ret {integer}  if ret < 0 ret is errno else ret is fd
function fs.open(path, flag, mode)
  if flag == nil then flag = 'r' end
  -- 0666
  if mode == nil then mode = 438 end
  return fs_native.open(path, flag, mode)
end

-- @example: local ret = fs.read(fd, buffer[, offset])
-- @param: fd {integer}
-- @param: buffer {ReadBuffer}
-- @param: offset {integer}
-- @return: ret {integer} if ret < 0 ret is errno else ret is read bytes
function fs.read(fd, buffer, offset)
  -- read from the current position
  if offset == nil then offset = -1 end
  return fs_native.read(fd, buffer, offset)
end

-- @example: local data, ret = fs.readFile(path[, flag, mode])
-- @param: path {string}
-- @param: flag {string}
-- @param: mode {integer}
-- @return: data {string}
-- @return: ret {integer} if ret < 0 ret is errno else ret is read bytes
function fs.readFile(path, flag, mode)
  if flag == nil then flag = 'r' end
  if mode == nil then mode = 438 end

  local fd = fs_native.open(path, flag, mode)
  if fd < 0 then return nil, fd end

  local stat, err
  stat, err = fs_native.fstat(fd)
  if err < 0 then
    fs_native.close(fd)
    return nil, err
  end

  local buffer
  buffer, err = ReadBuffer.new(stat.size)
  if err < 0 then
    fs_native.close(fd)
    return nil, err
  end
  
  local ret = fs_native.read(fd, buffer, -1)
  if ret < 0 then
    fs_native.close(fd)
    return nil, ret
  end

  fs_native.close(fd)
  return buffer:read(-1)
end

-- @example: local ret = fs.write(fd, data[, offset])
-- @param: fd {integer}
-- @param: data {string|buffer|table[array(string|buffer)]}
-- @param: offset {integer}
-- @return: ret {integer} if ret < 0 ret is errno else ret is write bytes
function fs.write(fd, data, offset)
  -- append
  if offset == nil then offset = -1 end
  return fs_native.write(fd, data, offset)
end

-- @example: local ret = fs.writeFile(path, data[, flag, mode])
-- @param: path {string}
-- @param: data {string|buffer|table[array(string|buffer)]}
-- @param: flag {string}
-- @param: mode {integer}
-- @return: ret {integer} if ret < 0 ret is errno else ret is write bytes
function fs.writeFile(path, data, flag, mode)
  if flag == nil then flag = 'w' end
  if mode == nil then mode = 438 end

  local fd = fs_native.open(path, flag, mode)
  if fd < 0 then return fd end

  return fs_native.write(fd, data, -1)
end

-- @example: local ret = fs.appendFile(path, data[, flag, mode])
-- @param: path {string}
-- @param: data {string|buffer|table[array(string|buffer)]}
-- @param: flag {string}
-- @param: mode {integer}
-- @return: ret {integer} if ret < 0 ret is errno else ret is write bytes
function fs.appendFile(path, data, flag, mode)
  if flag == nil then flag = 'a' end
  if mode == nil then mode = 438 end

  local fd = fs_native.open(path, flag, mode)
  if fd < 0 then return fd end

  return fs_native.write(fd, data, -1)
end

-- @example: local err = fs.unlink(path)
-- @param: path {string}
-- @return: err {integer}
function fs.unlink(path)
  return fs_native.unlink(path)
end

-- @example: local err = fs.rename(path, newpath)
-- @param: path {string}
-- @param: newpath {string}
-- @return: err {integer}
function fs.rename(path, newpath)
  return fs_native.rename(path, newpath)
end

-- @example: local err = fs.rmdir(path)
-- @param: path {string}
-- @return: err {integer}
function fs.rmdir(path)
  return fs_native.rmdir(path)
end

-- @example: local err = fs.mkdir(path[, mode])
-- @param: path {string}
-- @param: mode {integer}
-- @return: err {integer}
function fs.mkdir(path, mode)
  if mode == nil then mode = 511 end
  return fs_native.mkdir(path, mode)
end

-- @example: local err = fs.mkdtemp(temp)
-- @param: temp {string}
-- @return: err {integer}
function fs.mkdtemp(temp)
  return fs_native.mkdtemp(temp)
end

-- @example: local files, err= fs.readdir(path)
-- @param: path {string}
-- @return: files {table[array(string)]}
-- @return: err {integer}
function fs.readdir(path)
  return fs_native.readdir(path)
end

-- @example: local err = fs.sendfile(outfd, infd, offset, length)
-- @param: outfd {integer}
-- @param: infd {integer}
-- @param: offset {integer}
-- @param: length {integer}
-- @return: err {integer}
function fs.sendfile(outfd, infd, offset, length)
  return fs_native.senfile(outfd, infd, offset, length)
end

-- @example: local stat, err = fs.stat(path)
-- @param: path {string}
-- @return: stat {table}
--    stat = {
--      type = {integer} 
--        if type == fs.FILE
--        if type == fs.DIR
--        if type == fs.LINK
--        if type == fs.FIFO
--        if type == fs.SOCKET
--        if type == fs.CHAR
--        if type == fs.BLOCK
--      dev = {integer}
--      ino = {integer}
--      mode = {integer}
--      nlink = {integer}
--      uid = {integer}
--      gid = {integer}
--      rdev = {integer}
--      size = {integer}
--      blksize = {integer}
--      blocks = {integer}
--      atime = {integer}
--      mtime = {integer}
--      ctime = {integer}
--      birthtime = {integer}
--    }
-- @return: err {integer}
function fs.stat(path)
  return fs_native.stat(path)
end

-- @example: local stat, err = fs.fstat(fd)
-- @param: fd {integer}
-- @return: stat {table}
-- @return: err {integer}
function fs.fstat(fd)
  return fs_native.fstat(fd)
end

-- @example: local stat, err = fs.lstat(path)
-- @param: path {string}
-- @return: stat {table}
-- @return: err {integer}
function fs.lstat(path)
  return fs_native.lstat(path)
end

-- @example: local err = fs.link(path, newpath)
-- @param: path {string}
-- @param: newpath {string}
-- @return: err {integer}
function fs.link(path, newpath)
  return fs_native.link(path, newpath)
end

-- @example: local err = fs.symlink(path, newpath[, flag])
-- @param: path {string}
-- @param: newpath {string}
-- @param: flag {table}
--    flag = {
--      dir = {boolean}
--      junction = {boolean}
--    }
-- @return: err {integer}
function fs.symlink(path, newpath, flag)
  return fs_native.symlink(path, newpath, flag)
end

-- @example: local link, err = fs.readlink(path)
-- @param: path {string}
-- @return: link {string}
-- @return: err {integer}
function fs.readlink(path)
  return fs_native.readlink(path)
end

-- @example: local err = fs.chmod(path, mode)
-- @param: path {string}
-- @param: mode {integer}
-- @return: err {integer}
function fs.chmod(path, mode)
  return fs_native.chmod(path, mode)
end

-- @example: local err = fs.fchmod(fd, mode)
-- @param: fd {integer}
-- @param: mode {integer}
-- @return: err {integer}
function fs.fchmod(fd, mode)
  return fs_native.fchmod(fd, mode)
end

-- @example: local err = fs.chown(path, uid, gid)
-- @param: path {string}
-- @param: uid {integer}
-- @param: gid {integer}
-- @return: err {integer}
function fs.chown(path, uid, gid)
  return fs_native.chown(path, uid, gid)
end

-- @example: local err = fs.fchown(fd, uid, gid)
-- @param: fd {integer}
-- @param: uid {integer}
-- @param: gid {integer}
-- @return: err {integer}
function fs.fchown(fd, uid, gid)
  return fs_native.fchown(fd, uid, gid)
end

-- @example: local err = fs.utime(path, atime, mtime)
-- @param: path {string}
-- @param: atime {integer}
-- @param: mtime {integer}
-- @return: err {integer}
function fs.utime(path, atime, mtime)
  return fs_native.utime(path, atime, mtime)
end

-- @example: local err = fs.futime(fd, atime, mtime)
-- @param: fd {integer}
-- @param: atime {integer}
-- @param: mtime {integer}
-- @return: err {integer}
function fs.futime(fd, atime, mtime)
  return fs_native.futime(fd, atime, mtime)
end

-- @example: local err = fs.truncate(path[, length])
-- @param: path {string}
-- @param: length {integer}
-- @return: err {integer}
function fs.truncate(path, length)
  local fd = fs_native.open(path, 'r+', 438)
  if fd < 0 then return fd end

  if length == nil then length = 0 end
  local ret = fs_native.ftruncate(fd, length)
  local err = fs.native.close(fd)

  if ret < 0 then
    return ret
  else
    return err
  end
end

-- @example: local err = fs.ftruncate(fd[, length])
-- @param: fd {integer}
-- @param: length {integer}
-- @return: err {integer}
function fs.ftruncate(fd, length)
  if length == nil then length = 0 end
  return fs_native.ftruncate(fd, length)
end

-- @example: local err = fs.fsync(fd)
-- @param: fd {integer}
-- @return: err {integer}
function fs.fsync(fd)
  return fs_native.fsync(fd)
end

-- @example: local err = fs.fdatasync(fd)
-- @param: fd {integer}
-- @return: err {integer}
function fs.fdatasync(fd)
  return fs_native.fdatasync(fd)
end

local ReadStream = Readable:extend()

local read_options = {
  flag = 'r',
  mode = 438,
  bufferSize = 65536,
  fd = nil,
  offset = nil
}

local read_meta = {
  __index = read_options
}

-- @example: local err = ReadStream.init(self, path, options)
-- @param: self {table} child instance
-- @param: path {string}
-- @param: options {table} @ReadStream.init
--    local options = {
--      flag = {string}
--      mode = {integer}
--      bufferSize = {integer}
--      fd = {integer}
--      offset = {integer}
--    }
-- @return: err {integer}
function ReadStream:init(path, options)
  if not options then
    options = read_options
  else 
    setmetatable(options, read_meta)
  end

  local err = Readable.init(self, options.bufferSize)
  if err < 0 then return err end

  self.fd = options.fd
  if not self.fd then
    local ret = fs_native.open(path, options.flag, options.mode)
    if ret < 0 then return ret end
    self.fd = ret
  end

  self.read_bytes = 0
  self.offset = options.offset

  return 0
end

-- @example: local err = instance:_read()
-- @return: err {integer}
function ReadStream:_read()
  local ret = fs.read(self.fd, self.read_buffer, self.offset)
  if ret == 0 then ret = ERRNO.UV_EOF end
  if self.offset and ret > 0 then
    self.offset = self.offset + ret
  end

  return ret
end

-- @example: local err = instance:close()
-- @return: err {integer}
function ReadStream:close()
  if self.closed then return 0 end

  local err = 0
  if self.fd then
    err = fs_native.close(self.fd)
    self.fd = nil
  end

  self.read_buffer = nil
  self.closed = true
  return err
end

fs.ReadStream = ReadStream

-- @example: local readStream, err = fs.createReadStream(path, options)
-- @param: path {string}
-- @param: options {table} @ReadStream.init 
-- @return: readStream {table}
-- @return: err {integer}
function fs.createReadStream(path, options)
  return ReadStream:new(path, options)
end

local write_options = {
  flag = 'w',
  mode = 438,
  fd = nil,
  offset = nil
}

local write_meta = {
  __index = write_options
}

local WriteStream = Object:extend()

-- @example: local err = WriteStream.init(self, path, options)
-- @param: self {table} child instance
-- @param: path {string}
-- @param: options {table} @WriteStream.init
--    local options = {
--      flag = {string}
--      mode = {integer}
--      fd = {integer}
--      offset = {integer}
--    }
-- @return: err {integer}
function WriteStream:init(path, options)
  if not options then
    options = write_options
  else 
    setmetatable(options, write_meta)
  end

  self.fd = options.fd
  if not self.fd then
    local ret = fs_native.open(path, options.flag, options.mode)
    if ret < 0 then return ret end
    self.fd = ret
  end

  self.write_bytes = 0
  self.offset = options.offset
  self.closed = false

  return 0
end

-- @example: local err = instance:write(data)
-- @param: data {string|buffer|table[array(string|buffer)]}
-- @return: err {integer}
function WriteStream:write(data)
  if self.closed then error('closed, unavaliable') end

  local ret = fs.write(self.fd, data, self.offset)
  if ret > 0 then
    self.write_bytes = self.write_bytes + ret
    if self.offset then
      self.offset = self.offset + ret
    end
  end

  return ret
end

-- @example: local bytes = instance:bytesWritten()
-- @return: bytes {integer}
function WriteStream:bytesWritten()
  return self.write_bytes
end

-- @example: local err = instance:close()
-- @return: err {integer}
function WriteStream:close()
  if self.closed then return 0 end

  local err = 0
  if self.fd then
    err = fs_native.close(self.fd)
    self.fd = nil
  end
  self.closed = true

  return err
end

fs.WriteStream = WriteStream

-- @example: local writeStream, err = fs.createWriteStream(path, options)
-- @param: path {string}
-- @param: options {table} @WriteStream.init 
-- @return: writeStream {table}
-- @return: err {integer}
function fs.createWriteStream(path, options)
  return WriteStream:new(path, options)
end

return fs
