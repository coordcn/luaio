local Object = require('object')
local system = require('system')
local path = require('path')
local fs = require('fs')
local ERRNO = require('errno')

local strlib = require('strlib')
local SPACE8 = strlib.SPACE8
local LF = strlib.LF
local grequire = _G.require
local strbyte = string.byte
local strmatch = string.match

local modules_dir = path.dirname(process.execpath) .. '/modules/'
local lua_ext = '.lua'
local bin_ext = system.type == 'Windows' and '.dll' or '.so'
local modules_cache = {}

local Module = Object:extend()

local function get_real_path(file)
  local stat, err = fs.stat(file)
  if stat and stat.type == fs.FILE then
    return file
  end

  local real_path = file .. lua_ext
  stat, err = fs.stat(real_path)
  if stat and stat.type == fs.FILE then
    return real_path
  end

  real_path = file .. bin_ext
  stat, err = fs.stat(real_path)
  if stat and stat.type == fs.FILE then
    return real_path
  end
  
  return nil
end

local function err_nofile(name)
  local err = 'require(' .. name .. ') error: no such files' .. LF ..
              SPACE8 .. name .. LF ..
              SPACE8 .. name .. lua_ext .. LF ..
              SPACE8 .. name .. bin_ext .. LF
  return err
end

function Module:init(file_path, deps)
  self.path = file_path
  self.dir = path.dirname(file_path)
  self.deps = deps or {}
  return 0
end

function Module:load_from_path(real_path, deps)
  local ext = strmatch(real_path, '.(%.[^%./\\]*)$')
  local fn, err
  if ext == lua_ext then
    local module = Module:new(real_path, deps)
    local env = {
      module = module,
      require = function(name, version)
        return module:require(name, version)
      end
    }

    fn, err = loadfile(real_path, 'bt', setmetatable(env, { __index = _G }))
    if not fn then
      error(err .. '\n' .. debug.traceback())
    end
    return fn()
  elseif ext == bin_ext then
    fn, err = package.loadlib(real_path, fn_name)
    if not fn then
      error(real_path .. "#" .. fn_name .. ": " .. err)
    end
    return fn()
  else
    error()
  end
end

function Module:load_from_modules(name, version)
  version = version or self.deps[name]
  print(version)
  if version then
    -- redis@0.1.0
    local module_name = name .. '@' .. version
    if module_name and modules_cache[module_name] then
      return modules_cache[module_name], nil
    end
    
    local module_version_dir = path.join(modules_dir, name, version)
    local stat, err = fs.stat(module_version_dir)
    if err < 0 then
      error('fs.stat(' .. module_version_dir .. '): ' .. ERRNO.parser(err))
    end

    if stat.type ~= fs.DIR then
      error('require(' .. name .. ') error: ' .. module_version_dir .. 'is not a directory')
    end

    local module_version_package = path.join(module_version_dir, 'package.lua')
  print(module_version_package)
    local package_fn = assert(loadfile(module_version_package))
    local package = package_fn()
    if not package then
      error('no package')
    end

    local main = package.main
  print(main)
    if not main then
      error('no package.main')
    end

    local file = path.join(module_version_dir, main) .. lua_ext
    local module = Module:new(file, package.deps or self.deps)
    local env = {
      module = module,
      require = function(name, version)
        if not name or type(name) ~= 'string' then
          error('require(name) error: name must be string')
        end

        local ret, errstr = module:require(name, version)
        print(errstr)
        if ret then return ret end
        assert(errstr)
      end
    }
    local fn = assert(loadfile(file, 'bt', setmetatable(env, { __index = _G })))
    local ret = fn()
    if ret then return ret end
  else
    local module_dir = path.join(modules_dir, name)
    local stat, err = fs.stat(module_dir)
    if err < 0 then
      -- get from net
      error('fs.stat(' .. module_dir .. '): ' .. ERRNO.parse(err))
    end

    if stat.type ~= fs.DIR then
      error('require(' .. name .. ') error: ' .. module_dir .. ' is not a directory')
    end

    local files
    files, err = fs.readdir(module_dir) 
    if err < 0 then
      error('fs.readdir(' .. module_dir .. ') error: ' .. ERRNO.parser(err))
    end

  end
end

-- @brief require module
-- @example local ret = require(name[, version])
-- @param name {string}
--    cwd = process.cwd
--    1. native or internal module 'cwd/lib/process.lua'
--       require('process')
--    2. modules 'cwd/modules/redis/0.1.0/'    
--       require('redis'[, '0.1.0'])
--    3. relative path module
--       require('./test')
--    4. absolute path module
--       require('/home/public/lib/test')
-- @param version {string} '3.10.8'
-- @return ret {any}
function Module:require(name, version)
  if not name or type(name) ~= 'string' then
    error('require(name) error: name must be string' .. LF .. debug.traceback())
  end

  -- require('./test')
  local is_relative = strbyte(name, 1) == 46
  -- require('/home/public/lib/test')
  local is_absolute = path.isAbsolute(name)

  local real_path
  if is_relative then
    name = path.join(self.dir, name)
    real_path = get_real_path(name)
  elseif is_absolute then
    real_path = get_real_path(name)
  else
    if package.loaded[name] then
      return package.loaded[name]
    end

    -- not loaded native module
    if package.preload[name] then
      return require(name)
    end

    -- not loaded internal module
    local success, value = pcall(grequire, name)
    if success then return value end
    -- start with error e = 101
    if strbyte(value, 1) == 101 then
      error(value)
    end

    return self:load_from_modules(name, version)
  end

  if not real_path then
    error(err_nofile(name) .. debug.traceback())
  end

  if modules_cache[real_path] then
    return module_cache[real_path]
  end

  local ret = self:load_from_path(real_path, self.deps)
  if ret then module_cache[real_path] = ret end
  return ret
end

return Module
