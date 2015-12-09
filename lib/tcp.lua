local tcp_native = require('tcp_native')
local dns = require('dns')
local Object = require('object')
local Readable = require('readable')

local Socket = Readable:extend()

function Socket:init(handle, server)
end

local Server = Object:extend()

function Server:init(options)

end

function Server:listen(options)

end

function Server:close(options)

end

local tcp = {}

tcp.createServer = function(port, onconnect, options)
  return tcp_native.createServer(port, onconnect, options)
end

tcp.connect = function(port, host, options)
  local host_type = type(host)
  local address_type

  if host_type == 'table' then
    options = host
    host = nil
  elseif host_type == 'string' then
    address_type = tcp_native.isIP(host)
    if address_type == 0 then
    end
  end

  local family = options and options.family

  if family ~= 4 and family ~= 6 then
    family = 4
  end
  
  if address_type == 4 then
    family = 4
  elseif address_type == 6 then
    family = 6
  else
    if family ==  4 then
      if host then
        local ip, err = dns.resolve4(host)
        if err then error('tcp.connect(port[, host, options]) dns.resolve4(host) error: ' .. err.message) end
        host = ip[1]
      else
        host = '0.0.0.0'
      end
    else
      if host then
        local ip, err = dns.resolve6(host)
        if err then error('tcp.connect(port[, host, options]) dns.resolve6(host) error: ' .. err.message) end
        host = ip[1]
      else
        host = '::'
      end
    end
  end

  return tcp_native.connect(port, host, options)
end

tcp.isIP = function(ip)
  return tcp_native.isIP(ip)
end

return tcp
