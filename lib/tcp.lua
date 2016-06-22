local tcp_native = require('tcp_native')
local fs_native = require('fs_native')
local dns = require('dns')
local process = require('process')
local coro = require('coro')
local Object = require('object')
local Readable = require('readable')
local ERRNO = require('errno')

local Socket = Readable:extend()

-- @example: local err = Socket.init(self, size)
-- @param: self {table} child instance
-- @param: size {integer}
-- @return: err {integer}
function Socket:init(size)
  local buffer_size = size or 16384
  local err = Readable.init(self, buffer_size)
  if err < 0 then return err end

  self.errno = 0
  self.write_bytes = 0
  self.writing = 0
  self.closing = false
  self.closed = false

  return 0
end

function Socket:connect(port, host, timeout)
  if self.closed then error('closed, unavaliable') end
  if self.closing then error('closing, unavaliable') end

  if self.handle then
    error('socket has been connected, can not connect in this socket')
  end

  local handle = tcp_native.new()
  if not handle then return ERRNO.UV_ENOMEM end

  if timeout then
    handle:set_timeout(timeout)
  end

  if not host then host = '127.0.0.1' end
  local family = tcp_native.is_ip(host)

  local err
  if family ~= 0 then
    err = handle:connect(port, host)
    if err < 0 then
      handle:close()
      return err
    end
  else
    local ip4
    ip4, err = dns.resolve4(host)
    if err < 0 then
      local ip6
      ip6, err = dns.resolve6(host)
      if err < 0 then
        handle:close()
        return err
      else
        err = handle:connect(port, ip6[1])
        if err < 0 then
          handle:close()
          return err
        end
      end
    else
      err = handle:connect(port, ip4[1])
      if err < 0 then
        handle:close()
        return err
      end
    end
  end

  handle:set_read_buffer(self.read_buffer)
  self.fd = handle:fd()
  self.handle = handle

  return 0
end

-- @example: local err = instance:_read()
-- @return: err {integer}
function Socket:_read()
  if self.closing then error('closing, unavaliable') end 

  if not self.handle then
    error('not connected, please call socket:connect(port, host) first')
  end

  local err =  self.handle:read()
  self.errno = err

  return err
end

-- @example: local err = instance:write(data)
-- @param: data {string|buffer|table[array(string|buffer)]}
-- @param: bytes {integer} written bytes
-- @return: err {integer}
function Socket:write(data)
  if self.closed then error('closed, unavaliable') end
  if self.closing then error('closing, unavaliable') end

  if not self.handle then
    error('not connected, please call socket:connect(port, host) first')
  end

  local bytes, err = self.handle:write(data)

  if bytes > 0 then
    self.write_bytes = self.write_bytes + bytes
  end

  self.errno = err

  return bytes, err
end

-- @example: local err = instance:writeAsync(data)
-- @param: data {string|buffer|table[array(string|buffer)]}
-- @param: bytes {integer} written bytes
-- @return: err {integer}
function Socket:writeAsync(data)
  if self.closed then error('closed, unavaliable') end
  if self.closing then error('closing, unavaliable') end

  if not self.handle then
    error('not connected, please call socket:connect(port, host) first')
  end

  local bytes, err = self.handle:write_async(data)

  if bytes > 0 then
    self.write_bytes = self.write_bytes + bytes
  end

  self.errno = err

  return bytes, err
end

-- @example: bytes = instance:bytesWritten()
-- @return: bytes {integer}
function Socket:bytesWritten()
  return self.write_bytes
end

-- @example: local err = instance:sendfile(fd, offset, length)
-- @param: fd {integer}
-- @param: offset {integer}
-- @param: length {integer}
-- @return: err {integer}
function Socket:sendfile(fd, offset, length)
  if self.closed then error('closed, unavaliable') end
  if self.closing then error('closing, unavaliable') end

  if not self.fd then
    error('not connected, please call socket:connect(port, host) first')
  end

  local err = fs_natives.sendfile(self.fd, fd, offset, length)
  self.errno = err

  return err
end

-- @example: instance:setTimeout(ms)
-- @param: ms {integer}
function Socket:setTimeout(ms)
  if self.closed then error('closed, unavaliable') end
  if self.closing then error('closing, unavaliable') end

  if not self.handle then
    error('not connected, please call socket:connect(port, host) first')
  end

  self.handle:set_timeout(ms)
end

-- @example: local err = instance:setNodelay(enable)
-- @param: enable {boolean}
-- @return: err {integer}
function Socket:setNodelay(enable)
  if self.closed then error('closed, unavaliable') end
  if self.closing then error('closing, unavaliable') end

  if not self.handle then
    error('not connected, please call socket:connect(port, host) first')
  end

  return self.handle:set_nodelay(enable)
end

-- @example: local err = instance:setKeepalive(enable, idle)
-- @param: enable {boolean}
-- @param: idle {integer}
-- @return: err {integer}
function Socket:setKeepalive(enable, idle)
  if self.closed then error('closed, unavaliable') end
  if self.closing then error('closing, unavaliable') end

  if not self.handle then
    error('not connected, please call socket:connect(port, host) first')
  end

  return self.handle:set_keepalive(enable, idle)
end

-- @example: local addr = instance:localAddress()
-- @return: addr {table} @Socket:localAddress
--    local addr = {
--      family = {integer}
--      address = {string}
--      port = {integer}
--    }
function Socket:localAddress()
  if self._local_address then
    return self._local_address
  end

  if not self.handle or self.closed then
    return nil
  end

  local addr, err = self.handle:local_address()
  if err < 0 then return nil end

  self._local_address = addr
  return addr
end

-- @example: local addr = instance:remoteAddress()
-- @return: addr {table} @Socket:localAddress
function Socket:remoteAddress()
  if self._remote_address then
    return self._remote_address
  end

  if not self.handle or self.closed then
    return nil
  end

  local addr, err = self.handle:remote_address()
  if err < 0 then return nil end

  self._remote_address = addr
  return addr
end

function Socket:close()
  if self.closed then return end

  if self.errno < 0 then
    self:_close()
    return
  end

  self:_close()
end

function Socket:_close()
  if self.server then
    self.server:_decrease_connections()
  end

  -- do not set self.handle to nil
  -- if do this, handle(userdata) memory maybe free by lua 
  -- the close callback function will access invalid memory
  self.server = nil
  self.read_buffer = nil
  self.closed = true
  self:emit('close')

  if self.handle then
    self.handle:close()
  end
end

local Server = Object:extend()

local server_options = {
  host = nil,
  timeout = 0,
  nodelay = true,
  keepalive = false,
  keepidle = 0,
  backlog = 511,
  reuseport = false,
  bufferSize = 16384,
  maxConnections = 65535
}

local server_meta = {
  __index = server_options
}

-- @example: local err = Server.init(self, options)
-- @param port {integer}
-- @param onconnect {function}
--    function onconnect(socket)
--    end
-- @param options {table}
--    local options = {
--      host = {string}
--      timeout = {integer}
--      nodelay = {integer}
--      keepalive = {boolean}
--      keepidle = {integer}
--      backlog = {integer}
--      reuseport = {boolean}
--      bufferSize = {integer}
--      maxConnections = {integer}
--    }
function Server:init(port, onconnect, options)
  if not options then
    options = server_options
  else 
    setmetatable(options, server_meta)
  end

  self.quit = false
  self.connections = 0
  self.timeout = options.timeout
  self.nodelay  = options.nodelay
  self.keepalive = options.keepalive
  self.keepidle = options.keepidle
  self.buffer_size = options.bufferSize
  self.max_connections = options.maxConnections

  local handle = nil
  local handle_ = nil
  local err = 0

  local handle_ = tcp_native.new(true)
  if not handle_ then return ERRNO.UV_ENOMEM end

  if not options.host then
    err = handle_:bind(port, '::', options.reuseport)
    if err < 0 then
      handle_:close()
      handle = tcp_native.new()
      if not handle then return ERRNO.UV_ENOMEM end

      err = handle:bind(port, '0.0.0.0', options.reuseport)
      if err < 0 then
        handle:close()
        return err
      end
    else
      handle = handle_
    end
  else
    err = handle_:bind(port, options.host, options.reuseport)
    if err < 0 then
      handle_:close()
      return err
    end
    handle = handle_
  end

  -- one client connection one coroutine
  function _onconnect(client_handle)
    if self.closed or self.quit or ((self.connections + 1) > self.max_connections) then
      return
    end

    local socket, err = Socket:new(self.bufferSize)
    if not socket then
      client_handle:close()
      return 
    end

    client_handle:set_read_buffer(socket.read_buffer)
    socket.fd = client_handle:fd()
    socket.handle = client_handle
    socket.server = self
    socket:setTimeout(self.timeout)
    socket:setNodelay(self.nodelay)
    socket:setKeepalive(self.keepalive, self.keepidle)

    self.connections = self.connections + 1

    onconnect(socket)
    socket:close()
  end

  err = handle:listen(_onconnect, options.backlog)
  if err < 0 then
    handle:close()
    return err
  end

  self.handle = handle
  self.closed = false
  return 0
end

-- @example: local address, err = instance:address()
-- @return: address {table} @Socket:localAddress
-- @return: err {integer}
function Server:address()
  if self.closed then error('closed, unavaliable') end
  return self.handle:local_address()
end

-- @example: instance:_decrease_connections()
function Server:_decrease_connections()
  self.connections = self.connections - 1

  if self.quit then
    if self.connections == 0 then
      self:close()
      process.exit()
    end
  end
end

-- @example: instance:quit()
function Server:quit()
  self.quit = true
end

-- @example: instance:close()
function Server:close()
  if self.closed then return end
  self.closed = true
  self.handle:close()
end

local tcp = {}

tcp.createServer = function(port, onconnect, options)
  return Server:new(port, onconnect, options)
end

-- @example: local sokcet, err = tcp.connect(port, host, options)
-- @param: port {integer}
-- @param: host {string} hostname or IP
-- @param: options {table}
--    options = {
--      timeout = {integer}
--      buffer_size = {integer}
--    }
-- @return: socket {table}
-- @return: err {integer}
tcp.connect = function(port, host, options)
  options = options or {}
  local socket, err = Socket:new(options.buffer_size)
  if err < 0 then return nil, err end

  err = socket:connect(port, host, options.timeout)
  if err < 0 then
    socket:close()
    return nil, err
  end

  return socket, err
end

-- @example: local ret = tcp.isIP(ip)
-- @return: ret {integer}
--    if ret == 0 then unknown address family
--    if ret == 4 then IPv4
--    if ret == 6 then IPv6
tcp.isIP = function(ip)
  return tcp_native.is_ip(ip)
end

return tcp
