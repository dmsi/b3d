M = {}

local rot_speed = 10 + math.random()*120

function M.Start() 
  print ("[Lua.rotator_y].Start")
end

function M.Update()
  vec3 = b3d.vec3

  tr = globals.Transform
  p = tr.GetLocalPosition()
  dt = globals.Timer.dt
  tr.Rotate(vec3(0, rot_speed*dt, 0))
end

return M
