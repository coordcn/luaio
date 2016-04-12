local strlib_native = require('strlib_native')

local strgmatch = string.gmatch
local strmatch = string.match
local utf8char = utf8.char
local utf8codes = utf8.codes

local strlib = {}

strlib.LF = '\n'
strlib.CRLF = '\r\n'
strlib.SPACE8 = '        '

-- @brief split text by pattern
-- @example local parts, count = strlib.split(text, pattern[, mode])
-- @param text {string}
-- @param pattern {string}
-- @param mode {boolean|default: false or nil}
--    mode == false or nil => pattern is plain text or table
--    mode == true => pattern must be regexp
-- @return parts {table[array(string)]}
-- @return count {integer} parts length
function strlib.split(text, pattern, mode)
  if mode then
    local i = 0
    local parts = {}
    for part in strgmatch(text, pattern) do
      i = i + 1
      parts[i] = part
    end
    return parts, i
  else
    return strlib_native.split(text, pattern)
  end
end

function strlib.utf8len(text)
  return strlib_native.utf8len(text)
end

strlib.CODE = {
  NUL   = 0,      -- null                       \0
  SOH   = 1,      -- start of heading
  STX   = 2,      -- start of text
  ETX   = 3,      -- end of text
  EOT   = 4,      -- end of transimisssion
  ENQ   = 5,      -- enquiry
  ACK   = 6,      -- acknowledge
  BEL   = 7,      -- bell                       \a
  BS    = 8,      -- backspace                  \b
  TAB   = 9,      -- horizontal tab             \t
  LF    = 10,     -- NL line feed, new line     \n
  VT    = 11,     -- vertical tab               \v
  FF    = 12,     -- NP form fedd, new page     \f
  CR    = 13,     -- carriage return            \r
  SO    = 14,     -- shift out
  SI    = 15,     -- shift in
  DLE   = 16,     -- data link escape
  DC1   = 17,     -- device control 1
  DC2   = 18,     -- device control 2
  DC3   = 19,     -- device control 3
  DC4   = 20,     -- device control 4
  NAK   = 21,     -- negative acknowledge
  SYN   = 22,     -- synchronous idle
  ETB   = 23,     -- end of trans. block
  CAN   = 24,     -- cancel
  EM    = 25,     -- end of medium
  SUB   = 26,     -- substitute
  ESC   = 27,     -- escape                     \e
  FS    = 28,     -- file separator
  GS    = 29,     -- group separator
  RS    = 30,     -- record separator
  US    = 31,     -- unit separator
  [' '] = 32,     -- space
  ['!'] = 33,
  ['"'] = 34,
  ['#'] = 35,
  ['$'] = 36,
  ['%'] = 37,
  ['&'] = 38,
  ["'"] = 39,
  ['('] = 40,
  [')'] = 41,
  ['*'] = 42,
  ['+'] = 43,
  [','] = 44,
  ['-'] = 45,
  ['.'] = 46,
  ['/'] = 47,
  ['0'] = 48,
  ['1'] = 49,
  ['2'] = 50,
  ['3'] = 51,
  ['4'] = 52,
  ['5'] = 53,
  ['6'] = 54,
  ['7'] = 55,
  ['8'] = 56,
  ['9'] = 57,
  [':'] = 58,
  [';'] = 59,
  ['<'] = 60,
  ['='] = 61,
  ['>'] = 62,
  ['?'] = 63,
  ['@'] = 64,
  ['A'] = 65,
  ['B'] = 66,
  ['C'] = 67,
  ['D'] = 68,
  ['E'] = 69,
  ['F'] = 70,
  ['G'] = 71,
  ['H'] = 72,
  ['I'] = 73,
  ['J'] = 74,
  ['K'] = 75,
  ['L'] = 76,
  ['M'] = 77,
  ['N'] = 78,
  ['O'] = 79,
  ['P'] = 80,
  ['Q'] = 81,
  ['R'] = 82,
  ['S'] = 83,
  ['T'] = 84,
  ['U'] = 85,
  ['V'] = 86,
  ['W'] = 87,
  ['X'] = 88,
  ['Y'] = 89,
  ['Z'] = 90,
  ['['] = 91,
  ['\\'] = 92,
  [']'] = 93,
  ['^'] = 94,
  ['_'] = 95,
  ['`'] = 96,
  ['a'] = 97,
  ['b'] = 98,
  ['c'] = 99,
  ['d'] = 100,
  ['e'] = 101,
  ['f'] = 102,
  ['g'] = 103,
  ['h'] = 104,
  ['i'] = 105,
  ['j'] = 106,
  ['k'] = 107,
  ['l'] = 108,
  ['m'] = 109,
  ['n'] = 110,
  ['o'] = 111,
  ['p'] = 112,
  ['q'] = 113,
  ['r'] = 114,
  ['s'] = 115,
  ['t'] = 116,
  ['u'] = 117,
  ['v'] = 118,
  ['w'] = 119,
  ['x'] = 120,
  ['y'] = 121,
  ['z'] = 122,
  ['{'] = 123,
  ['|'] = 124,
  ['}'] = 125,
  ['~'] = 126,
  DEL   = 127
}

return strlib
