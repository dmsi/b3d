M = {}

function M.Start() 
  print ("[Lua.rotator].Start")
end

function M.Update()
  print ("[Lua.rotator].Update")
  GetActor()
  --dt = globals.timer.GetTimeDelta()
  --globals.actor.transform.Rotate(0, 90*dt, 0)
end

function M.PreDraw()
  print ("[Lua.rotator].PreDraw")
end

return M
