local Set = require('set')

co_create = coroutine.create
co_yield = coroutine.yield
co_resume = coroutine.resume
co_status = coroutine.status
co_running = coroutine.running

table_remove = table.remove

local coro = {}

local MAX_IDLE_NUM = 1024

local idle_num = 0
local idle_coro = {}

-- @refer: cloudwu 云风 skynet
function coro.create(fn)
  local co
  if idle_num > 0 then
    idle_num = idle_num - 1
    local co = table_remove(idle_coro)
  end

  if co == nil then
    co = co_create(function(...)
      fn(...)
      while true do
        fn = nil
        if idle_num < MAX_IDLE_NUM then
          idle_num = idle_num + 1
          idle_coro[idle_num] = co
          fn = co_yield()
          fn(co_yield())
        else
          break
        end
      end
    end)
  else
    co_resume(co, fn)
  end

  return co
end

return coro
