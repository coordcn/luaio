local split = require('strlib').split

local string_match = string.match

-- @example local dir, name, ext = string.match(str, reg)
-- dirname = dir
-- basename = name .. ext
-- extname = ext
local posix_split_reg = '^(.*)/([^/]*)(%.[^%./]*)(/*)$'

local posix = {}

posix.sep = '/'
posix.delimiter = ':'

function posix.dirname = function(path)
  local dir, name, ext = string_match(path, posix_split_reg)
  return dir or '.'
end

function posix.basename = function(path, extname)
  local dir, name, ext = string_match(path, posix_split_reg)
  return ext == extname and name or name .. ext
end

function posix.extname = function(path)
  local dir, name, ext = string_match(path, posix_split_reg)
  return ext
end

function posix.parse = function(path)
  return string_match(path, posix_split_reg)
end

function posix.normalize = function(path)
end

function posix.join = function(...)
  local parts = {...}
end

function posix.resolve = function(...)
  local parts = {...}
end

-- @example local dir, name, ext = string.match(str, reg)
-- dirname = dir
-- basename = name .. ext
-- extname = ext
local windows_split_reg = '^(.*)[/\\]([^/\\]*)(%.[^%./\\]*)([/\\]*)$'

local windows = {}

windows.sep = '\\'
windows.delimiter = ';'

function windows.dirname = function(path)
  local dir, name, ext = string_match(path, windows_split_reg)
  return dir or '.'
end

function windows.basename = function(path, extname)
  local dir, name, ext = string_match(path, windows_split_reg)
  return ext == extname and name or name .. ext
end

function windows.extname = function(path)
  local dir, name, ext = string_match(path, windows_split_reg)
  return ext
end

function windows.parse = function(path)
  return string_match(path, windows_split_reg)
end

function windows.normalize = function(path)
end

function windows.join = function(...)
  local parts = {...}
end

function windows.resolve = function(...)
  local parts = {...}
end
