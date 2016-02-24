local Emitter = require('emitter')
local system = require('system');
local process_native = require('process_native')
local signal_native = require('signal_native')

local emitter = Emitter:new()
local signals = {}
local pids = {}

local process = {}

-- @brief: the pid of the process
-- @example: local pid = process.pid
-- @return: pid {integer}
process.pid = process_native.pid

-- @brief: the command line arguments
-- @example：local argv = process.argv
-- @return：argv {table[array(string)]}
process.argv = __ARGV__

local execpath = process_native.execpath() or __ARGV__[1]

-- @brief: the absolute pathname of the excutable that started the process
-- @example：local execpath = process.execpath
-- @return：execpath {string}
process.execpath = execpath

-- @brief: the current working directory of the process
-- @example：local cwd = process.cwd
-- @return：cwd {string}
process.cwd = process_native.cwd()


-- @brief: the current working directory of the process
-- @example：local ret = process.fork(file, options)
-- @param: file {string}
-- @param: options {table}
--    local options = {
--      args = {table[array(string)]}
--      forever = {boolean}
--      cpu = {integer}
--      onexit = {function}
--      onrestart = {function}
--      uid = {integer}
--      gid = {integer}
--      detached = {boolean}
--    }
--    
--    -- may be used in logger
--    function onexit(pid, file, status, signal)
--      print('module: ' .. file .. ' exit')
--      print('pid: ' .. pid )
--      print('error: ' .. ERRNO.parse(status))
--      print('signal: ' .. signal )
--    end
--
--    -- may be used in logger
--    function onrestart(pid, file)
--      -- error
--      if pid < 0 then
--        print('module: ' .. file .. ' restart error')
--        print('error:' .. ERRNO.parser(pid))
--        return
--      end
--
--      print('module: ' .. file .. ' restart ok')
--      print('pid:' .. pid)
--    end
-- @return: ret {integer} if ret < 0 => error
--                        if ret > 0 => pid
function process.fork(file, options)
  if type(file) ~= 'string' then
    error('process.fork(file, options) error: file must be string')
  end

  options = options or {}
  if type(options) ~= 'table' then
    error('process.fork(file, options) error: options must be table')
  end

  local args = options.args or {}
  if type(args) ~= 'table' then
    error('process.fork(file, options) error: options.args must be table')
  end

  for i, v in ipairs(args) do
    if type(v) ~= 'string' then
      error('process.fork(file, options) error: options.args[' .. i .. '] must be string')
    end
  end

  table.insert(args, 1, file)

  local forever = options.forever
  if forever and type(forever) != 'boolean' then
    error('process.fork(file, options) error: options.forever must be boolean')
  end

  local cpu = options.cpu
  if cpu and type(cpu) != 'number' then
    error('process.fork(file, options) error: options.cpu must be integer')
  end

  local onexit = options.onexit
  if onexit and type(onexit) != 'function' then
    error('process.fork(file, options) error: options.onexit must be function')
  end

  local onrestart = options.onrestart
  if onrestart and type(onrestart) != 'function' then
    error('process.fork(file, options) error: options.restart must be function')
  end

  local uid = options.uid
  if uid and type(uid) != 'number' then
    error('process.fork(file, options) error: options.uid must be integer')
  end

  local gid = options.gid
  if gid and type(gid) != 'number' then
    error('process.fork(file, options) error: options.gid must be integer')
  end

  local detached = options.detached
  if detached and type(detached) != 'boolean' then
    error('process.fork(file, options) error: options.detached must be boolean')
  end

  local function _onexit(pid, status, signal)
    local pid_info = pids[pid]
    if onexit then
      onexit(pid, pid_info.file, status, signal)
    end

    pids[pid] = nil

    if pid_info.forever then
      local _pid = process_native.spawn(pid_info.options)
      if _pid > 0 then
        if pid_info.cpu then
          process_native.setaffinity(_pid, pid_info.cpu)
        end

        pids[_pid] = pid_info
      end
    end
  end

  local opts = {
    file = execpath,
    args = args,
    onexit = _onexit,
    uid = uid,
    gid = gid,
    detached = detached
  }

  local pid = process_native.spawn(opts)
  if pid > 0 then
    if cpu then
      -- setaffinity is just suggestion(chinese english, ugly...)
      process_native.setaffinity(pid, cpu)
    end

    pids[pid] = {
      file = file,
      forever = forever,
      cpu = cpu,
      options = opts
    }
  end
  
  return pid
end

-- @brief: execute command
-- @example：local ret = process.exec(command)
-- @param: command {string}
-- @return: ret {integer} if ret < 0 => error
--                        if ret > 0 => pid
function process.exec(command)
  if type(command) ~= 'string' then
    error('process.exec(command, args) error: command must be string')
  end

  local file, args;
  if system.type = 'Windows' then
    file = 'cmd.exe'
    args = {'/s', '/c', '"' .. command .. '"'}
  else
    file = '/bin/sh'
    args = {'/c', command}
  end
 
  return process_native.spawn({file = file, args = args})
end

function process.exit(signal)
  emitter:emit('exit', signal)
  process_native.exit(signal)
end

function process.abort()
  process_native.abort()
end

function process.kill(pid, signal)
  return process_native.kill(pid, signal)
end

function process.on(name, callback)
  local signum = process_native[name]
  if type(signum) == 'number' and not signals[name] then
    local signal = signal_native.new()
    signals[name] = signal
    signal:start(signum, function()
      emitter:emit(name)
    end)
  end

  emitter:on(name, callback)
end

function process.off(name, callback)
  if not callback then
    local signal = signals[name]
    if signal then
      signal:stop()
      signal:close()
    end
  end

  emitter:off(name, callback)
end

return process
