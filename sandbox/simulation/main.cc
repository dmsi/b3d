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

#include "b3d.h"
#include "my/all.h"
#include "phys.h"
#include "actions.h"
  
int main(int argc, char* argv[]) {
  using glm::vec3;
  Scene scene;

  LOG_SCOPE_F(INFO, "Helo blyat!");

  // Initialize application.
  AppContext::Init(1280, 720, "Sandbox [b3d]", Profile("3 3 core"));
  AppContext::Instance().display.ShowCursor(false);
  int width = AppContext::Instance().display.GetWidth();
  int height = AppContext::Instance().display.GetHeight();

  // Setup main and shadowmap rendertargets
  Cfg<RenderTarget>(scene, "rt.screen", 2000)
    . Tags("onscreen")
    . Clear(.8, .8, .8, 1)
    . Done();
  
  auto shadow_tex = Cfg<RenderTarget>(scene, "rt.shadowmap", 0) 
    . Camera("camera.shadowmap")
    . Tags("shadow-caster")
    . Type(FrameBuffer::kTexture2D)
    . Resolution(width, height)
    . Layer(Layer::kDepth, Layer::kReadWrite)
    . Done()
    ->GetLayerAsTexture(0, Layer::kDepth);
  
  auto atmosphere_tex = Cfg<RenderTarget>(scene, "rt.atmosphere", 100)
    . Tags("atmosphere")
    . Type(FrameBuffer::kTexture2D)
    . Resolution(width, height)
    . Layer(Layer::kColor, Layer::kReadWrite)
    . Done()
    ->GetLayerAsTexture(0, Layer::kColor);
  
  // Light source
  auto sun = Cfg<Light>(scene, "light.sun", Light::kDirectional)
    . EulerAngles(40, -120, 0)
    . Done();

  // Main camera
  Cfg<Camera>(scene, "camera.main")
    . Perspective(60, (float)width/height, .1, 1500)
    . Position(0, 1, 4)
    . Action<FlyingCameraController>(20)
    . Done();
  
  // Shadowmap camera
  int w = 25, h = 25, d = 10; // the shadowcube, bigger the cube => lower the quality
  auto cam_shadowmap = Cfg<Camera>(scene, "camera.shadowmap")
    . Ortho(-w, h, w, -h, -d, d*2)
    . Action<ShadowMapCameraController>(sun, 10)
    . Done();

  /////////////////////////////////////////////////////////////////////////////
  // Compose scene
  /////////////////////////////////////////////////////////////////////////////

  Surface cube_srf  {Color(.5, 0, 0, 1), Color(.5, 0, 0, 1), Color(0), 0};
  Surface floor_srf { };

  auto cube_shader = MaterialLoader::Load("Sandbox/simulation/object.mat");
  auto cube_mesh = MeshLoader::Load("Assets/unity_cube.dsm"); 
  
  for (int i = 0; i < 15; ++i) {
    auto cube = Cfg<Actor>(scene, "actor.cube" + std::to_string(i))
      . Model(cube_mesh, cube_shader)
      . Texture(0, shadow_tex)
      . Texture(1, atmosphere_tex)
      . Position(i*1.8,  1,  0)
      . EulerAngles(30, 30, 0)
      . Action<Rotator>(glm::vec3(30, 30, 30))
      . Action<ShadowShader>(sun, cam_shadowmap, cube_srf)
      . Done();
    
    //Cfg<Actor>(scene, "actor.debug" + std::to_string(i))
    //  . Action<BoundsVisual>(cube)
    //  . Done();
  }
  
  auto floor = Cfg<Actor>(scene, "actor.floor")
    . Model("Assets/plane.dsm", "Sandbox/simulation/object.mat")
    . Scale(200, 1, 200)
    . Texture(0, shadow_tex)
    . Texture(1, atmosphere_tex)
    . Action<ShadowShader>(sun, cam_shadowmap, floor_srf)
    . Done();


  Cfg<Actor>(scene, "actor.skydome")
    . Model("Assets/sphere.dsm", "Sandbox/simulation/skydome_perez.mat")
    . Done();
  
  // Fps meter.
  Cfg<Actor>(scene, "actor.fps.meter")
    . Action<FpsMeter>()
    . Done();
  
  // Main loop. Press ESC to exit.
  do {
    AppContext::BeginFrame();
    scene.Update();
    scene.Draw();
    AppContext::EndFrame();
  } while (AppContext::Running());

  // Cleanup and close the app.
  AppContext::Close();
  return 0;
}
