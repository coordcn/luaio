local Object = require('object')
local system = require('system')
local path = require('path')
local fs = require('fs')
local ERRNO = require('errno')

local strlib = require('strlib')
local SPACE8 = strlib.SPACE8
local LF = strlib.LF
local grequire = _G.require

local modules_dir = process.cwd .. '/modules/'
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

local function err_nofile(name, filename)
  filename = filename or name
  local errstr = 'require(\'' .. name .. '\') error: no such files' .. LF ..
                 SPACE8 .. filename .. LF ..
                 SPACE8 .. filename .. lua_ext .. LF ..
                 SPACE8 .. filename .. bin_ext .. LF
  return errstr
end

function Module:init(file_path, package)
  package = package or {}
  self.path = file_path
  self.dir = path.dirname(file_path)
  self.package = package
  local deps = package.deps or {}
  self.deps = deps
  return 0
end

function Module:load_from_path(real_name, real_path, deps)
  local ext = path.extname(real_path)
  if ext == lua_ext then
    local module = Module:new(real_path, deps)
    local env = {
      module = module,
      require = function(name, version)
        return module:require(name, version)
      end
    }

    local fn, err = loadfile(real_path, 'bt', setmetatable(env, { __index = _G }))
    if not fn then
      error('require(\'' .. real_name .. '\') error: ' .. LF .. err .. LF .. debug.traceback())
    end
    return fn()
  elseif ext == bin_ext then
    local fn_name = 'luaopen_' .. real_name
    local fn, err = package.loadlib(real_path, fn_name)
    if not fn then
      error('require(\'' .. real_name .. '\') error: ' .. LF .. err .. LF .. debug.traceback())
    end
    return fn()
  else
    local errstr = 'require(\'' .. real_name .. '\') error: extname must be ' .. lua_ext ..
                   ' or ' .. bin_ext .. LF .. 
                   debug.traceback()
    error(errstr)
  end
end

-- TODO 模块加载
-- version = major.minor.patch(releaseinfo)
-- 优先在版本库中匹配版本字符串完全相同的版本，版本不完全匹配，则给出警告。
-- 如无完全相同的版本匹配，忽略releaseinfo，比较major.minor.patch。
-- major.minor.patch都有的，三者完全相同的版本匹配
-- major.minor，匹配major.minor版本下patch最高的版本
-- major，匹配major版本下minor.patch最高的版本
-- nil或''，匹配major.minor.patch最高的版本
-- 本地加载
-- 远程加载（git）
-- 本地编译
function Module:load_from_modules(name, version)
  version = version or self.deps[name]

end

local DOT_CODE = strlib.CODE['.']
local SLASH_CODE = strlib.CODE['/']
local BACKSLASH_CODE = strlib.CODE['\\']

local function path_is_relative(path)
  local c1, c2, c3 = path:byte(1, 3)
  -- './'
  if c1 == DOT_CODE then
    if c2 == SLASH_CODE or c2 == BACKSLASH_CODE then
      return true
    end

    -- '../'
    if c2 == DOT_CODE then
      if c3 == SLASH_CODE or c3 == BACKSLASH_CODE then
        return true
      end
    end
  end

  return false
end

local grequire_not_found_reg = '^module .* not found:'

-- @brief require module
-- @example local ret = require(name[, version])
-- @param name {string}
--    cwd = process.cwd
--    1. native or internal module 'cwd/lib/process.lua'
--       require('process')
--    2. modules 'cwd/modules/redis/0.1.0/'    
--       require('redis'[, '0.1.0'])
--       modules 'cwd/modules/db/redis/0.1.0/'    
--       require('db/redis'[, '0.1.0'])
--    3. relative path module
--       require('./test')
--       require('../../test')
--    4. absolute path module
--       require('/home/public/lib/test')
--       require('e:/public/lib/test')
-- @param version {string} '3.10.8'
-- @return ret {any}
function Module:require(name, version)
  if not name or type(name) ~= 'string' then
    error('require(name[, version]) error: name must be string' .. LF .. debug.traceback())
  end

  local real_path
  local resolved_path
  if path_is_relative(name) then
    resolved_path = path.resolve(self.dir, name)
    real_path = get_real_path(resolved_path)
  elseif path.isAbsolute(name) then
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
    if not value:match(grequire_not_found_reg) then
      local errstr = 'require(' .. name .. ') error:' .. LF .. 
                     SPACE8 .. value .. LF .. 
                     debug.traceback()
      error(errstr)
    end

    return self:load_from_modules(name, version)
  end

  if not real_path then
    error(err_nofile(name, resolved_path) .. debug.traceback())
  end

  if modules_cache[real_path] then
    return modules_cache[real_path]
  end

  local ret = self:load_from_path(name, real_path, self.deps)
  if ret then modules_cache[real_path] = ret end
  return ret
end

return Module
