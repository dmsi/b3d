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

struct SetOverlayUniform: public Action {
  glm::vec4 color;
  bool      invert_uv;

  SetOverlayUniform(std::shared_ptr<Transformation> transform) 
    : Action(transform), color(1, 1, 1, 1), invert_uv(false) {}

  void PreDraw() override {
    if (auto mtrl = transform->GetActor().GetComponent<Material>()) {
      mtrl->SetUniform("Color", color);
      int int_invert_uv = invert_uv;
      mtrl->SetUniform("Invert_uv", int_invert_uv);
    }
  }

  void Setup(const glm::vec4& color, bool invert_uv) {
    this->color = color;
    this->invert_uv = invert_uv;
  }
};

struct SetAxisUniform : public Action {
  Color color = Color(1, 1, 1, 1);

  SetAxisUniform(std::shared_ptr<Transformation> transform, 
                 Color color) 
    : Action(transform) {
    this->color = color;
  }

  void PreDraw() override {
    if (auto mtrl = transform->GetActor().GetComponent<Material>()) {
      mtrl->SetUniform("color", color);
    }
  }
};

int main(int argc, char* argv[]) {
  Scene scene;

  // Step 1. Initialize application.
  AppContext::Init(1280, 720, "Water [b3d]", Profile("3 3 core"));
  AppContext::Instance().display.ShowCursor(false);
  int width = AppContext::Instance().display.GetWidth();
  int height = AppContext::Instance().display.GetHeight();
  
  //SpawnActor("Assets/unity_cube.dsm", "Assets/texture.mat");

  // Step 4. Add 'sun' light source and its debug visualisation. 
  //std::shared_ptr<Light> sun = scene.Add<Light>("light.sun");
  //sun->transform->SetLocalPosition(0, 5, 5);
  //sun->transform->SetLocalEulerAngles(-210, 90, 0);
  // Cfg<Scene>::CaptureScene(scene); => makes shared ptr of scene 
  // ... 
  // Cfg<Scene>::ReleaseScene();
  Cfg<Light>(scene, "light.sun")
    . Position(0, 5, 6)
    . EulerAngles(-210, 90, 0)
    . Color(1, 0, 0, 1) 
    . Done();
  
  Cfg<Actor>(scene, "actor.axis.x")
    . Mesh("Assets/arrow.dsm")
    . Material("Assets/axis.mat")
    . EulerAngles(0, 90, 0)
    . Action<SetAxisUniform>(Color(1, 0, 0, 0))
    . Done();
  
  Cfg<Actor>(scene, "actor.axis.y")
    . Mesh("Assets/arrow.dsm")
    . Material("Assets/axis.mat")
    . EulerAngles(-90, 0, 0)
    . Action<SetAxisUniform>(Color(0, 1, 0, 0))
    . Done();
  
  Cfg<Actor>(scene, "actor.axis.z")
    . Mesh("Assets/arrow.dsm")
    . Material("Assets/axis.mat")
    . Action<SetAxisUniform>(Color(0, 0, 1, 0))
    . Done();
  
  Cfg<Actor>(scene, "actor.axis.origin")
    . Mesh("Assets/sphere.dsm")
    . Material("Assets/axis.mat")
    . Scale(.3, .3, .3)
    . Action<SetAxisUniform>(Color(.8, .8, 0, 0))
    . Done();
  
  Cfg<Actor>(scene, "actor.fps.meter")
    . Action<FpsMeter>()
    . Done();

  
  // Cfg<Camera> ... Cfg<Light> ???
  // Step 5. Configure the main camera. 
  auto camera = scene.Add<Camera>("camera.main");
  camera->SetPerspective(60, 1.0f*width/height, .1, 150);
  camera->transform->SetLocalPosition(0, 0, 5);
  //camera->AddAction<FlyingCameraController>()->moving_speed = 150;
  camera->AddAction<FlyingCameraController>()->moving_speed = 5;

  // Step 2. Setup RenderTarget and FrameBuffer.
  //scene.AddRenderTarget("rt.screen", 1000)
  //  ->Configure("tags", Tags({"onscreen"}))
  //  ->Configure("clear", Color(.8, .8, .8, 1))
  //  ->Done();
  Cfg<RenderTarget>(scene.AddRenderTarget("rt.screen", 1000))
    . Tags("onscreen")
    . Clear(.8, .8, .8, .8)
    . Done();

  int w = 100, h = 100;
  auto xyz_tex = Cfg<RenderTarget>(scene, "rt.xyz", 100)
    . Camera      ("camera.main")
    . Tags        ("xyz1", "aa", "bb", "xyz", "xxx")
    . Type        (FrameBuffer::kTexture2D)
    . Resolution  (w, h)
    . Layer       (Layer::kColor, Layer::kReadWrite)
    . Layer       (Layer::kDepth, Layer::kWrite)
    . Clear       (.4, .4, .4, 1)
    . Done        ()
    ->GetLayerAsTexture(0, Layer::kColor);

  scene.Add<Actor>("action.overlay.xyz")
    ->AddAction<MakeOverlayArea>()
    ->Setup(&scene, xyz_tex, MakeOverlayArea::kTop3, false);

  /// testing
  //Cfg<Actor>(scene.Add<Actor>("actor.>>>"))
  //  . Action<LoadModel>(
  //      "Assets/the_long_object.dsm",
  //      "Assets/oooookey.mat",
  //    )
  //  . Action<SetUniforms>(
  //      "sssss",
  //      glm::vec3(1)
  //    )
  //  . Material("....")
  //  . Model("....")
  //  . Position(xxx)
  //  . Scale(yyy)
  //  ->Done();
  
  //glViewport(0, 0, 400*1.67, 400); 
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
