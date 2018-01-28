//
// This source file is a part of borsch.3d
//
// Copyright (C) borsch.3d team 2017-2018
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include <iostream>
#include <memory>
#include <limits>

#include "b3d.h"
#include "myactions/all.h"
#include "myhelpers/all.h"

int main(int argc, char* argv[]) {
  Scene scene;

  // Step 1. Initialize application.
  AppContext::Init(1280, 720, "Skybox cubemap [b3d]", Profile("3 3 core"));
  AppContext::Instance().display.ShowCursor(false);
  int width = AppContext::Instance().display.GetWidth();
  int height = AppContext::Instance().display.GetHeight();
  
  // Step 2. Setup RenderTarget.
  Cfg<RenderTarget>(scene.AddRenderTarget("rt.screen"))
    . Tags("onscreen")
    . Clear(.4, .4, .4, 1)
    . Done();

  // Step 3. Compose the scene. 
  Cfg<Camera>(scene, "camera.main")
    . Perspective(90, (float)width/height, .1, 500)
    . Position(0, 0, 1)
    . Action<FlyingCameraController>(5)
    . Done();

  auto tex = Cfg<Actor>(scene, "actor.skybox")
    . Model("Assets/blender_cube.dsm", "Assets/cubemap_env.mat")
    . Done()
      ->GetComponent<MeshRenderer>()->GetMaterial()->GetTexture(0);
  
  Cfg<Actor>(scene, "actor.object1")
    . Model("Assets/sphere.dsm", "Assets/cubemap_rr_surface_vs.mat")
    . Texture(0, tex)
    //. Action<Rotator>(glm::vec3(0, 30, 0))
    . Done();

  Cfg<Actor>(scene, "actor.fps.meter")
    . Action<FpsMeter>()
    . Done();

  // Step 4. Main loop. Press ESC to exit.
  do {
    AppContext::BeginFrame();
    scene.Update();
    scene.Draw();
    AppContext::EndFrame();
  } while (AppContext::Running());

  // Step 5. Cleanup and close the app.
  AppContext::Close();
  return 0;
}
