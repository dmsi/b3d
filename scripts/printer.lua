M = {}

function M.Start() 
  print ("[Lua.printer].Start")
end

function M.Update()
  --print ("[Lua.printer].Update")
  trn = globals.transform
  dt = b3d.Timer.GetTimeDelta()
  --p = trn:GetLocalPosition()
  --p.y = -1
  --print("pos     = ", p.x, p.y, p.z)
  --trn:SetLocalPosition(p)
  --print("new pos = ", p.x, p.y, p.z)
  --globals.transform:SetLocalPosition(p)
  trn:Rotate(b3d.vec3(0, 90 * dt, 0))
  --print (b3d.Timer.dt, b3d.Timer.time)
end

function M.PreDraw()
  --print ("[Lua.printer].PreDraw")
end

return M
