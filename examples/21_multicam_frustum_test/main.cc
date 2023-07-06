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

// Tests if global position is in camera1 and/or camera2 frustrum and 
// assigns color based on the test result.
struct FrustrumTest : public Action {
  std::shared_ptr<Camera> camera1;
  std::shared_ptr<Camera> camera2;

  FrustrumTest(std::shared_ptr<Transformation> t, 
               std::shared_ptr<Camera> c1, std::shared_ptr<Camera> c2) 
    : Action(t), camera1(c1), camera2(c2) {
    assert(camera1);
    assert(camera2);
  }

  void PreDraw() override {
    if (auto m = GetActor().GetComponent<Material>()) {
      auto pos = transform->GetGlobalPosition();
      auto& frustum1 = camera1->GetFrustum();
      auto& frustum2 = camera2->GetFrustum();
      bool test1 = frustum1.TestPoint(pos);
      bool test2 = frustum2.TestPoint(pos);

      Color c((float)test1, 0, (float)test2, 1);
      m->SetUniform("tint", c);
    }
  }
};

// Visualises camera's frustum
struct FrustumVisual : public Action {
  std::shared_ptr<Camera> camera;

  FrustumVisual(std::shared_ptr<Transformation> t, std::shared_ptr<Camera> c) 
    : Action(t), camera(c) {

    // Make unit (NDC) cube
    auto mesh = std::make_shared<Mesh>();
    mesh->vertices = {
      2.0f * glm::vec3(-0.5, -0.5, -0.5),
      2.0f * glm::vec3(0.5, -0.5, -0.5),
      2.0f * glm::vec3(0.5,  0.5, -0.5),
      2.0f * glm::vec3(-0.5,  0.5, -0.5),
      2.0f * glm::vec3(-0.5, -0.5,  0.5),
      2.0f * glm::vec3(0.5, -0.5,  0.5),
      2.0f * glm::vec3(0.5,  0.5,  0.5),
      2.0f * glm::vec3(-0.5,  0.5,  0.5)
    };
   
    // Indexing for drawing as lines
    // TODO use geometry shader in order to turn points to smooth thick lines
    mesh->indices = {
      0, 1, 2, 3, 
      4, 5, 6, 7, 
      0, 4, 1, 5, 
      2, 6, 3, 7,
      0, 3, 1, 2, 
      4, 7, 5, 6
    };

    auto mf = GetActor().GetComponent<MeshFilter>();
    auto mr = GetActor().GetComponent<MeshRenderer>();
    if (!mf) mf = GetActor().AddComponent<MeshFilter>();
    if (!mr) mr = GetActor().AddComponent<MeshRenderer>();
    mf->SetMesh(mesh);
    mr->primitive = MeshRenderer::kPtLines;
  }

  void PreDraw() override {
    assert(camera);
    glm::mat4 p, v;
    camera->GetProjectionMatrix(p);
    camera->GetViewMatrix(v);
    if (auto m = GetActor().GetComponent<Material>()) {
      m->SetUniform("target_projection", p);
      m->SetUniform("target_view", v);
    }
  }
};


int main(int argc, char* argv[]) {
  using glm::vec3;
  using T = std::vector<std::string>;
  Scene scene;

  // Initialize application.
  AppContext::Init(1280, 720, "Multicamera frustum test [b3d]", Profile("3 3 core"));
  AppContext::Instance().display.ShowCursor(false);
  int width = AppContext::Instance().display.GetWidth();
  int height = AppContext::Instance().display.GetHeight();

  /////////////////////////////////////////////////////////////////////////////
  // Setup render targets: screen, maincam, cam1 and cam2
  /////////////////////////////////////////////////////////////////////////////
  Cfg<RenderTarget>(scene, "rt.screen", 2000)
    . Tags("onscreen")
    . Clear(.4, .4, .4, 1)
    . Done();
  
  auto maincam_tex = Cfg<RenderTarget>(scene, "rt.maincam", 0) 
    . Camera("camera.main")
    . Tags("maincam")
    . Type(FrameBuffer::kTexture2D)
    . Resolution(width, height) 
    . Layer(Layer::kColor, Layer::kReadWrite)
    . Layer(Layer::kDepth, Layer::kWrite)
    . Clear(.8, .8, .8, 1)
    . Done()
    ->GetLayerAsTexture(0, Layer::kColor);
  
  auto cam1_tex = Cfg<RenderTarget>(scene, "rt.cam1", 1) 
    . Camera("cam1")
    . Tags("cam1")
    . Type(FrameBuffer::kTexture2D)
    . Resolution(width, height) 
    . Layer(Layer::kColor, Layer::kReadWrite)
    . Layer(Layer::kDepth, Layer::kWrite)
    . Clear(.8, .8, .8, 1)
    . Done()
    ->GetLayerAsTexture(0, Layer::kColor);
  
  auto cam2_tex = Cfg<RenderTarget>(scene, "rt.cam2", 2) 
    . Camera("cam2")
    . Tags("cam2")
    . Type(FrameBuffer::kTexture2D)
    . Resolution(width, height) 
    . Layer(Layer::kColor, Layer::kReadWrite)
    . Layer(Layer::kDepth, Layer::kWrite)
    . Clear(.8, .8, .8, 1)
    . Done()
    ->GetLayerAsTexture(0, Layer::kColor);

  /////////////////////////////////////////////////////////////////////////////
  // Set our cameras - main, cam1 and cam2
  /////////////////////////////////////////////////////////////////////////////

  // Main, at altitude of 20, looks straight down, interactive.
  Cfg<Camera>(scene, "camera.main")
    . Perspective(60, (float)width/height, .1, 80) 
    . Position(0, 20, 0)
    . EulerAngles(-90, 0, 0)
    . Action<FlyingCameraController>(5)
    . Done();

  // cam1, narrow-angle perspective, rotating around Y
  auto cam1 = Cfg<Camera>(scene, "cam1")
    . Perspective(20, (float)width/height, 1, 10) 
    . Position(0, 0, -4)
    . EulerAngles(0, 180, 0)
    . Action<Rotator>(vec3(0, 10, 0))
    . Done();
 
  // cam1 origin visualiser
  Cfg<Actor>(scene, "cam1.origin.visual")
    . Model("Assets/arrow.dsm", "Assets/Materials/arrow.mat")
    . Tags(0, T{"maincam"})
    . EulerAngles(0, 180, 0)
    . Parent(cam1)
    . Done();

  // cam1 frustum visualiser
  Cfg<Actor>(scene, "cam1.frustum.visual")
    . Material("Assets/Materials/frustum.mat")
    . Tags(0, T{"maincam"})
    . Action<FrustumVisual>(cam1)
    . Done();
  
  // cam2, ortho camera, rotating around Y
  auto cam2 = Cfg<Camera>(scene, "cam2")
    . Ortho(-1, 1, 1, -1, 1, 10)
    . Position(0, 0, 4)
    . Action<Rotator>(vec3(0, 10, 0))
    . Done();
 
  // cam2 origin visualiser
  Cfg<Actor>(scene, "cam2.origin.visual")
    . Model("Assets/arrow.dsm", "Assets/Materials/arrow.mat")
    . Tags(0, T{"maincam"})
    . EulerAngles(0, 180, 0)
    . Parent(cam2)
    . Done();

  // cam2 frustum visualiser
  Cfg<Actor>(scene, "cam2.frustum.visual")
    . Material("Assets/Materials/frustum.mat")
    . Tags(0, T{"maincam"})
    . Action<FrustumVisual>(cam2)
    . Done();
 
  /////////////////////////////////////////////////////////////////////////////
  // Spawn objects which position is going to be frustum-tested
  /////////////////////////////////////////////////////////////////////////////
  auto obj_mesh = MeshLoader::Load("Assets/arrow.dsm");
  auto obj_mtrl = MaterialLoader::Load("Assets/Materials/tint.mat");
  for (int i = 0; i < 60; ++i) {
    Cfg<Actor>(scene, "actor.obj" + std::to_string(i))
      . Model(obj_mesh, obj_mtrl)
      . Tags(0, T{"maincam", "cam1", "cam2"})
      . Position(Math::Random(-10, 10), 0, Math::Random(-10, 10))
      . EulerAngles(-90, 0, 0)
      . Action<FrustrumTest>(cam1, cam2)
      . Done();
  }

  /////////////////////////////////////////////////////////////////////////////
  // Displays
  /////////////////////////////////////////////////////////////////////////////

  // Note: displays should have different copies of the same materials, because
  // they are going to use different textures.
  Cfg<Actor>(scene, "actor.screen.left")
    . Model("Assets/screen.dsm", "Assets/Materials/overlay_texture.mat")
    . Texture(0, maincam_tex)
    . Scale(0.49, 0.49, 1)
    . Position(-0.5, 0, 0)
    . Done();
  
  Cfg<Actor>(scene, "actor.screen.righttop")
    . Model("Assets/screen.dsm", "Assets/Materials/overlay_texture.mat")
    . Texture(0, cam1_tex)
    . Scale(0.49, 0.49, 1)
    . Position( 0.5,  0.5, 0)
    . Done();
  
  Cfg<Actor>(scene, "actor.screen.rightbottom")
    . Model("Assets/screen.dsm", "Assets/Materials/overlay_texture.mat")
    . Texture(0, cam2_tex)
    . Scale(0.49, 0.49, 1)
    . Position( 0.5, -0.5, 0)
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
