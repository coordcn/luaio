local strlib = require('strlib')

local split = strlib.split
local SLASH_CODE = strlib.CODE['/']
local BACKSLASH_CODE = strlib.CODE['\\']

local function normalize_array(parts)
  local skip = 0

  for i = #parts, 1, -1 do
    local part = parts[i]
    if part == '.' then
      table.remove(parts, i)
    elseif part == '..' then
      table.remove(parts, i)
      skip = skip + 1
    elseif skip > 0 then
      table.remove(parts, i)
      skip = skip - 1
    end
  end

  return skip
end

-- @example local dir, name, ext = string.match(str, reg)
-- dirname = dir
-- basename = name .. ext
-- extname = ext
local posix_split_reg = '^(.*)/([^/]*)(%.[^%./]*)(/*)$'

local posix = {}

posix.sep = '/'
posix.delimiter = ':'

function posix.dirname(path)
  local dir, name, ext = path:match(posix_split_reg)
  return dir or '.'
end

function posix.basename(path, extname)
  local dir, name, ext = path:match(posix_split_reg)
  return ext == extname and name or name .. ext
end

-- @TODO maybe rewrite by c
function posix.extname(path)
  local dir, name, ext = path:match(posix_split_reg)
  return ext
end

function posix.isAbsolute(path)
  if not path or path == '' then return false end
  return path:byte(1) == SLASH_CODE  
end

function posix.parse(path)
  return path:match(posix_split_reg)
end

-- @TODO maybe rewrite by c
function posix.normalize(path)
  if not path or path == '' then return '.' end
  
  local len = #path;
  if len == 1 then return path end

  local is_absolute = path:byte(1) == SLASH_CODE
  local tailing_slash = path:byte(len) == SLASH_CODE
  local parts = split(path, '/')
  local skip = normalize_array(parts)
  local filepath = table.concat(parts, '/')

  if is_absolute then
    if #filepath == 0 then return '/' end
    filepath = '/' .. filepath
  else
    if skip == 0 then
      if #filepath == 0 then filepath = '.' end
    else 
      local temp = string.rep('..', skip, '/')
      if #filepath == 0 then
        filepath = temp
      else
        filepath = temp .. '/' .. filepath
      end
    end
  end
    
  if trailing_slash then
    filepath = filepath .. '/'
  end

  return filepath
end

function posix.join(...)
  local parts = {...}
  local filepath = table.concat(parts, '/')
  return posix.normalize(filepath)
end

function posix.resolve(...)
  local parts = {...}
  local len = #parts;

  local is_absolute = false;
  local resolved_path = ''
  for i = len, 1, -1 do
    local path = parts[i]
    if path ~= '' then
      resolved_path = path .. '/' .. resolved_path
      if path:byte(1) == SLASH_CODE then
        is_absolute = true
        break
      end
    end
  end

  if not is_absolute then
    return posix.join(process.cwd, resolved_path)
  end

  return posix.normalize(resolved_path)
end

-- @example local dir, name, ext = string.match(str, reg)
-- dirname = dir
-- basename = name .. ext
-- extname = ext
local windows_split_reg = '^(.*)[/\\]([^/\\]*)(%.[^%./\\]*)([/\\]*)$'
local drive_reg = '^[%a]:'
local UNC_reg = '^[\\/][\\/][^?\\/.]'
local UNC_long_reg = '^[\\/][\\/]?[\\/]UNC'
local drive_long_reg = '^[\\/][\\/]?[\\/][%a]:'

local windows = {}

windows.sep = '\\'
windows.delimiter = ';'

function windows.dirname(path)
  local dir, name, ext = path:match(windows_split_reg)
  return dir or '.'
end

function windows.basename(path, extname)
  local dir, name, ext = path:match(windows_split_reg)
  return ext == extname and name or name .. ext
end

function windows.extname(path)
  local dir, name, ext = path:match(windows_split_reg)
  return ext
end

local WINDOWS_ROOT_TYPE_NO_ROOT         = 1
local WINDOWS_ROOT_TYPE_DRIVE           = 2
local WINDOWS_ROOT_TYPE_DRIVE_RELATIVE  = 3
local WINDOWS_ROOT_TYPE_UNC             = 4
local WINDOWS_ROOT_TYPE_UNC_LONG        = 5
local WINDOWS_ROOT_TYPE_DRIVE_LONG      = 6

local function get_root(path)
  if not path or path == '' then
    return WINDOWS_ROOT_TYPE_NO_ROOT, nil
  end

  local len = #path
  if len == 1 then
    return WINDOWS_ROOT_TYPE_NO_ROOT, nil
  end

  local drive = path:match(drive_reg)
  if drive then
    if len == 2 then
      return WINDOWS_ROOT_TYPE_DRIVE, drive
    end

    local next_char = path:byte(3)
    if next_char == '\\' or next_char == '/' then
      return WINDOWS_ROOT_TYPE_DRIVE, drive
    else
      return WINDOWS_ROOT_TYPE_DRIVE_RELATIVE, drive
    end
  end

  if len == 2 then
    return WINDOWS_ROOT_TYPE_NO_ROOT, nil
  end

  local UNC = path:match(UNC_reg)
  if UNC then
    return WINDOWS_ROOT_TYPE_UNC, UNC:gsub('/', '\\')
  end

  local UNC_long = path:match(UNC_long_reg)
  if UNC_long then
    return WINDOWS_ROOT_TYPE_UNC_LONG, UNC_long:gsub('/', '\\')
  end

  local drive_long = path:match(drive_long_reg)
  if drive_long then
    return WINDOWS_ROOT_TYPE_DRIVE_LONG, drive_long:gsub('/', '\\')
  end

  return WINDOWS_ROOT_TYPE_NO_ROOT, nil
end

function windows.isAbsolute(path)
  local root_type, root = get_root(path)
  if root_type == WINDOWS_ROOT_TYPE_NO_ROOT or root_type == WINDOWS_ROOT_TYPE_DRIVE_RELATIVE then
    return false
  else
    return true
  end
end

function windows.parse(path)
  return path:match(windows_split_reg)
end

function windows.normalize(path)
  if not path or path == '' then return '.' end
 
  local root_type, root = get_root(path)
  if root_type ~= WINDOWS_ROOT_TYPE_NO_ROOT then
    path = path:sub(#root + 1)
  end

  path = path:gsub('\\', '/')
  local tailing_slash = path:byte(#path) == SLASH_CODE
  local parts = split(path, '/')
  local skip = normalize_array(parts)
  local filepath = table.concat(parts, '\\')

  if root_type ~= WINDOWS_ROOT_TYPE_NO_ROOT then
    if skip == 0 then
      if #filepath == 0 then filepath = '.' end
    else 
      local temp = string.rep('..', skip, '\\')
      if #filepath == 0 then
        filepath = temp
      else
        filepath = temp .. '\\' .. filepath
      end
    end
  else
    if root_type == WINDOWS_ROOT_TYPE_DRIVE_RELATIVE then
      if #filepath == 0 then return root end
      filepath = root .. filepath
    else
      if #filepath == 0 then return root .. '\\' end
      filepath = root .. '\\' .. filepath
    end
  end
    
  if trailing_slash then
    filepath = filepath .. '\\'
  end

  return filepath
end

function windows.join(...)
  local parts = {...}
  local filepath = table.concat(parts, '/')
  return windows.normalize(filepath)
end

function windows.resolve(...)
  local parts = {...}
  local len = #parts;

  local is_absolute = false;
  local resolved_path = ''
  for i = len, 1, -1 do
    local path = parts[i]
    if path ~= '' then
      resolved_path = path .. '/' .. resolved_path
      local root_type, root = get_root(path)
      if root_type ~= WINDOWS_ROOT_TYPE_NO_ROOT then
        is_absolute = true
        break
      end
    end
  end

  if not is_absolute then
    return windows.join(process.cwd, resolved_path)
  end

  return windows.normalize(resolved_path)
end

function windows.makelong(path)
  if not path then return '' end

  local resolved_path = windows.resolve(path)
  local root_type, root = get_root(resolved_path)
  if root_type == WINDOWS_ROOT_TYPE_DRIVE or root_type == WINDOWS_ROOT_TYPE_UNC then
    return resolved_path
  else
    return path
  end
end

if system.type == 'Windows' then
  windows.posix = posix
  windows.windows = windows
  return windows
else
  posix.posix = posix
  posix.windows = windows
  return posix
end
