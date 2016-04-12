local co_create = coroutine.create
local co_yield = coroutine.yield
local co_running = coroutine.running
local co_resume = coroutine.resume
local co_status = coroutine.status
local unpack = table.unpack

local parallel = {}

function parallel.all(funcs, limit)
  if type(funcs) ~= 'table' then
    error('parallel.all(funcs, limit) error: funcs must be table(array[function])')
  end

  local num = #funcs
  if limit == nil then limit = num end
  if type(limit) ~= 'number' then
    error('parallel.all(funcs, limit) error: limit must be number')
  end
  if limit > num then limit = num end

  local parent_co = co_running()
  local results = {}
  local index = 0
  local flag = false
  local completed = 0
  local running = 0

  function collect(index, ...)
    results[index] = {...}
    completed = completed + 1
    if completed == running then
      co_resume(parent_co)
    end
  end

  while true do
    while running < limit do
      index = index + 1
      local co = co_create(function(func, index)
        collect(index, func())
      end)
      co_resume(co, funcs[index], index)

      if co_status(co) == 'dead' then
        completed = completed - 1
      else
        running = running + 1
      end

      if index == num then
        flag = true
        break 
      end
    end

    if running > 0 then co_yield() end
    if flag then break end
    running = 0
    completed = 0
  end

  return results
end

function parallel.map(args, func, limit)
  if type(args) ~= 'table' then
    error('parallel.map(args, func, limit) error: args must be table(array[array])')
  end

  if type(func) ~= 'function' then
    error('parallel.map(args, func, limit) error: fun must be function')
  end

  local num = #args
  if limit == nil then limit = num end
  if type(limit) ~= 'number' then
    error('parallel.map(args, func, limit) error: limit must be number')
  end
  if limit > num then limit = num end

  local parent_co = co_running()
  local results = {}
  local index = 0
  local flag = false
  local completed = 0
  local running = 0

  function collect(index, ...)
    results[index] = {...}
    completed = completed + 1
    if completed == running then
      co_resume(parent_co)
    end
  end

  while true do
    while running < limit do
      index = index + 1
      local co = co_create(function(arg, index)
        if type(arg) == 'table' then
          collect(index, func(unpack(arg)))
        else
          collect(index, func(arg))
        end
      end)
      co_resume(co, args[index], index)
      if co_status(co) == 'dead' then
        completed = completed - 1
      else
        running = running + 1
      end

      if index == num then
        flag = true
        break 
      end
    end

    if running > 0 then co_yield() end
    if flag then break end
    running = 0
    completed = 0
  end

  return results
end

return parallel
