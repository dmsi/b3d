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

struct SetClipPlaneUniform: public Action {
  std::shared_ptr<Actor> plane;

  SetClipPlaneUniform(std::shared_ptr<Transformation> t, 
                      std::shared_ptr<Actor> p) 
    : Action(t), plane(p) {}

  void PreDraw() override {
    if (auto mtrl = transform->GetActor().GetComponent<Material>()) {
      auto pos = plane->transform->GetLocalPosition();
      float clip_bias = .01;
      glm::vec4 clip_plane(0, 1, 0, -pos.y + clip_bias);
      mtrl->SetUniform("clip_plane", clip_plane);
    }
  }
};

int main(int argc, char* argv[]) {
  using glm::vec3;
  Scene scene;
  float water_level = 0.5;
  
  // Step 1. Initialize application.
  AppContext::Init(1280, 720, "Surface reflections [b3d]", Profile("3 3 core"));
  AppContext::Instance().display.ShowCursor(false);
  int width = AppContext::Instance().display.GetWidth();
  int height = AppContext::Instance().display.GetHeight();
  
  // Step 2. Setup onscreen RT 
  Cfg<RenderTarget>(scene, "rt.screen", 1)
    . Tags("onscreen")
    . Clear(.4, .8, .8, 1)
    . Done();
  
  // Step 2.1. Setup offscreen RT for reflections and get reflections
  // texture placeholder.
  auto reflection_texture = Cfg<RenderTarget>(scene, "rt.reflection", 0)
    . Camera("camera.main.reflection")
    . Tags("reflection")
    . Type(FrameBuffer::kTexture2D)
    . Resolution(width/2, height/2)
    . Layer(Layer::kColor, Layer::kReadWrite)
    . Layer(Layer::kDepth, Layer::kWrite)
    . Clear(.4, .8, .8, 1)
    . Done()
    ->GetLayerAsTexture(0, Layer::kColor);
  
  // Step 3. Setup the reflections destination 
  auto mirror = Cfg<Actor>(scene, "actor.mirror")
    . Model("Assets/plane.dsm", "Assets/reflection_dst.mat")
    . Position(0, water_level, 0)
    . Texture(0, reflection_texture)
    . Done();

  // Step 3.2. Setup the reflections sources
  Cfg<Actor>(scene, "actor.knight1")
    . Model("Assets/knight.dsm", "Assets/reflection_src.mat")
    . Position(-2.5, .5,  -2.5)
    . Action<SetClipPlaneUniform>(mirror)
    . Action<Rotator>(vec3(0, -90, 0))
    . Done();

  Cfg<Actor>(scene, "actor.knight2")
    . Model("Assets/knight.dsm", "Assets/reflection_src.mat")
    . Position(2.5,  .5,  2.5)
    . Action<SetClipPlaneUniform>(mirror)
    . Action<Rotator>(vec3(0, 90, 0))
    . Done();

  Cfg<Actor>(scene, "actor.cube")
    . Model("Assets/unity_cube.dsm", "Assets/reflection_src.mat")
    . Position(0,  .5,  0)
    . Action<SetClipPlaneUniform>(mirror)
    . Action<Rotator>(vec3(90, 90, 90))
    . Done();
  
  // Step 4. The FPS meter
  Cfg<Actor>(scene, "actor.fps_meter")
    . Action<FpsMeter>()
    . Done();
  
  // Step 4.1. Add overlay screen with reflection texture.
  Cfg<Actor>(scene, "actor.overlay")
    . Action<MakeOverlayArea>(reflection_texture, MakeOverlayArea::kBottom0, true)
    . Done();
  
  // Step 5. Configure the main camera. 
  auto camera = Cfg<Camera>(scene, "camera.main")
    . Perspective(60, (float)width/height, .1, 100)
    . Position(-5,  2,  5)
    . EulerAngles(-15, -45, 0)
    . Done();

  // Step 5.1. Configure the reflection camera
  Cfg<Camera>(scene, "camera.main.reflection")
    . Action<ReflectCamera>(camera, water_level)
    . Done();

  // Step 6. Main loop. Press ESC to exit.
  do {
    AppContext::BeginFrame();
    scene.Update();
    scene.Draw();
    AppContext::EndFrame();
  } while (AppContext::Running());

  // Step 7. Cleanup and close the app.
  AppContext::Close();
  return 0;
}
