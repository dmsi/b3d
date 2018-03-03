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

// http://prideout.net/blog/?p=48

#include "b3d.h"
#include "my/all.h"

// Changes tesselation levels with time
struct TessShader : public Action {
  float tess_level_inner = 1;
  float tess_level_outer = 1;

  float dt_inner; // tess level change speed levels/second
  float dt_outer; // tess level change speed levels/second

  TessShader(std::shared_ptr<Transformation> t, float dti, float dto)
    :   Action(t), dt_inner(dti), dt_outer(dto)
  {}

  void Update() override {
    tess_level_inner += dt_inner * GetTimer().GetTimeDelta();
    tess_level_outer += dt_outer * GetTimer().GetTimeDelta();

    if (tess_level_inner > 10) {
      dt_inner = -abs(dt_inner);
      tess_level_inner = 10;
    } 
    if (tess_level_inner < 0) {
      dt_inner = abs(dt_inner);
      tess_level_inner = 0;
    }
    
    if (tess_level_outer > 10) {
      dt_outer = -abs(dt_outer);
      tess_level_outer = 10;
    } 
    if (tess_level_outer < 0) {
      dt_outer = abs(dt_outer);
      tess_level_outer = 0;
    }
  }

  void PreDraw() override {
    if (auto m = GetActor().GetComponent<Material>()) {
      m->SetUniform("TessLevelInner", tess_level_inner);
      m->SetUniform("TessLevelOuter", tess_level_outer);
    }
  }
};

int main(int argc, char* argv[]) {
  using glm::vec3;
  Scene scene;

  // Initialize application.
  AppContext::Init(1280, 720, "Triangle tesselation [b3d]", Profile("4 0 core"));
  AppContext::Instance().display.ShowCursor(false);
  int width = AppContext::Instance().display.GetWidth();
  int height = AppContext::Instance().display.GetHeight();

  // Configure render targets
  Cfg<RenderTarget>(scene, "rt.screen", 2000)
    . Tags("onscreen")
    . Clear(.8, .8, .8, 1)
    . Done();

  // Spawn icosahedron and instruct MeshRenderer to use patch as
  // the draw primitive.
  Cfg<Actor>(scene, "actor.tess")
    . Model("Assets/icosahedron.dsm", "Assets/tri_tesselation.mat")
    . Action<TessShader>(1, 2)
    . Done()
    ->GetComponent<MeshRenderer>()
    ->SetPatch(3);

  // Main camera
  Cfg<Camera>(scene, "camera.main")
    . Perspective(60, (float)width/height, .1, 100)
    . Position(0, 0, 4)
    . Action<FlyingCameraController>(5)
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
