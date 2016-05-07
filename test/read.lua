local fs = require('fs')
local ERRNO = require('errno')

local read_stream, err = fs.createReadStream('./http_parser/CURL_GET.txt')

print(ERRNO.parse(err))

local str, err = read_stream:read(20)

print(ERRNO.parse(err))
print(str)

str, err = read_stream:read(99)

print(ERRNO.parse(err))
print(str)

str, err = read_stream:read(9)

print(ERRNO.parse(err))
print(str)
