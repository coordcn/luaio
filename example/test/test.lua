local real_path = 'D:/../test/t-_%~!@#$&^*()_+= t/ ..//////...  ../////sanme.t t..tt.tt/////////////'
local real_path1 = 'D:/\\../\\test/t-_%~!@#$&^*()_+= t/\\ ..//////\\\\\\...  ../////\\\\sanme.t t..tt.tt/////////////\\\\\\'
local posix_path_split_reg = '^(/?)(.*/)([^/]*)(%.[^%./]*)(/*)$'
local windows_path_split_reg = '^(.*)[/\\]([^/\\]*)(%.[^%./\\]*)([/\\]*)$'

--local posix_path_split_reg = '(.*)(%.[^%./]*)(/*)$'
local root, dir, name, ext
local start = system.hrtime()
for i = 1, 1000 do
 root, dir, name, ext = string.match(real_path1, windows_path_split_reg)
end
local endtime = system.hrtime()

print((endtime - start) / 1000000)

print(root)
print(dir)
print(name)
print(ext)

local split = require('strlib').split

local test = 'wo shi zhong guo ren!'
local test1 = '我 是 中 国 人 ！'

local parts, count
start = system.hrtime()
for i = 1, 1000 do
parts, count = split(test1, ' ')
end
endtime = system.hrtime()
print((endtime - start) / 1000000)

for i = 1, count do
print(parts[i])
end
require('./run.lua')
