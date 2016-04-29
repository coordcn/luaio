local color = {}

local colors = {
  black   = "0;30",
  red     = "0;31",
  green   = "0;32",
  yellow  = "0;33",
  blue    = "0;34",
  magenta = "0;35",
  cyan    = "0;36",
  white   = "0;37",
  B        = "1;",
  Bblack   = "1;30",
  Bred     = "1;31",
  Bgreen   = "1;32",
  Byellow  = "1;33",
  Bblue    = "1;34",
  Bmagenta = "1;35",
  Bcyan    = "1;36",
  Bwhite   = "1;37"
}

local function _color(color_name)
  return "\27[" .. (colors[color_name] or "0") .. "m"
end

local colorize

if system.type == 'Windows' then
  colorize = function(color_name, string, reset_name)
    return string
  end
else
  colorize = function(color_name, string, reset_name)
    return _color(color_name) .. string .. _color(reset_name)
  end
end

function color.red(string)
  return colorize('red', string, 'white')
end

function color.green(string)
  return colorize('green', string, 'white')
end

function color.yellow(string)
  return colorize('yellow', string, 'white')
end

function color.blue(string)
  return colorize('blue', string, 'white')
end

function color.magenta(string)
  return colorize('magenta', string, 'white')
end

function color.cyan(string)
  return colorize('cyan', string, 'white')
end

return color
