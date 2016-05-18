local fs = require('fs')
local stream = fs.createReadStream('read.txt')

while true do

local data, bytes = stream:readline()

print(data)
print(bytes)

if bytes < 0 then break end

end
