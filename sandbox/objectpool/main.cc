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

struct Lifetime : public Action {
  float life;

  Lifetime(std::shared_ptr<Transformation> t, float l)
    : Action(t), life(l) {}

  void Update() override {
    life -= GetTimer().GetTimeDelta();
    if (life <= 0) {
      GetActor().Die();
    }
  }
};

struct Spawner : public Action {
  float timeout;

  Spawner(std::shared_ptr<Transformation> t)
    : Action(t), timeout(Math::Random(1, 3)) {}

  void Update() override {
    timeout -= GetTimer().GetTimeDelta();
      //std::cerr << timeout << std::endl;
    if (timeout < 0) {
      //std::cerr << "yay!" << std::endl;
      //timeout = Math::Random(1/, .3);
      timeout = 1/25.;

      auto& self = *((ActorPool*)&GetActor());
      auto a = self.Get(true);
      if (!a) return;
      a->AddAction<Lifetime>(Math::Random(5, 15));
      a->transform->SetLocalPosition(Math::Random(-5, 5), 0, Math::Random(-5, 5));
      if (Math::Random() < .3) {
        a->AddAction<Rotator>(glm::vec3(0, 90, 0));
      }
    }
  }
};

int main(int argc, char* argv[]) {
  using glm::vec3;
  Scene scene;

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
  
  auto pool = Cfg<ActorPool>(scene, "actor.knight.pool", 100)
    . Model("Assets/knight.dsm", "Assets/texture.mat")
    . Action<Spawner>()
    . Done();

  //while (true) {
  //  auto k1 = pool->Get(); 
  //  if (!k1) break;
  //  k1->transform->SetLocalPosition(Math::Random(-20, 20), 0, Math::Random(-20, 20));
  //  if (Math::Random() < .3) {
  //    k1->AddAction<Rotator>(glm::vec3(0, 90, 0));
  //    k1->AddAction<Lifetime>(Math::Random(10, 20));
  //  }
  //}

  Cfg<Actor>(scene, "floor")
    . Model("Assets/plane.dsm", "Assets/texture.mat")
    . Done();

  // Main camera
  Cfg<Camera>(scene, "camera.main")
    . Perspective(60, (float)width/height, 1, 8000) 
    . Position(0, 1, 4)
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
