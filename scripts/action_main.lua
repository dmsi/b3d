-- This needs to be pushed from the C++
modules = {}
globals = {}

function Bind()
  --This is not necessary and even adds a little overhead, but makes LUA API
  --more consistent and clean. globals._transform:XXX() can be use directly.
  --Havent found any better way of doing this
  globals.Transform = {}
  function closure(v)
    f = function(...)
      return v(globals._transform, ...)
    end
    return f
  end

  for k, v in pairs(getmetatable(globals._transform)) do
    if string.match(tostring(k), "^[A-Z]") then
      fname = tostring(k)
      globals.Transform[fname] = closure(v) 
    end
  end
end

-- Start single module, it is required in case when new
-- modules are added in runtime
function StartModule(m)
  if modules[m] ~= nil then
    if modules[m].Start ~= nil then modules[m].Start() end
  end
end

-- Start all modules
function Start() 
  for i, m in pairs(modules) do
    if m.Start ~= nil then m.Start() end
  end
end

-- Update all modules
function Update()
  for i, m in pairs(modules) do
    if m.Update ~= nil then m.Update() end
  end
end

-- PreDraw all modules
function PreDraw()
  for i, m in pairs(modules) do
    if m.PreDraw ~= nil then m.PreDraw() end
  end
end

-- Register module
function RegisterModule(m)
  modules[m] = require(m)
end

-- Deregister module
function DeregisterModule(m)
  modules[m] = nil
end

-- Discover and print registered modules information
function DiscoverModules()
  print('Discovering modules...')
  for i, m in pairs(modules) do
    print (i, m)
    for k, v in pairs(m) do
      print (" -> ", k, v)
    end
  end

  print('Discovering globals...')
  for k, v in pairs(globals) do
    print (k, v)
  end
end

