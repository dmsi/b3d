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
  
  //Cfg<Actor>(scene, "actor.terrain")
  //  . Model("Assets/screen.dsm", "Assets/texture.mat")
  //  . Done();

  auto a = scene.Add<Actor>("actor.aaaa");
  a->AddComponent<MeshRenderer>()->SetMaterial(MaterialLoader::Load("Assets/color.mat"));

  /*
  auto mf = a->AddComponent<BypassMeshFilter>();
  using L = AttributeLayout<false, glm::vec3>;
  std::vector<uint8_t> buf(L::Stride() * 3);
  BufferView buf_view(&buf[0], buf.size());
  L::Set<0>(buf_view, 0,  glm::vec3(-1, -1,  0));
  L::Set<0>(buf_view, 1,  glm::vec3( 1, -1,  0));
  L::Set<0>(buf_view, 2,  glm::vec3( 0,  1,  0));

  mf->Upload<L>(0, 3, 3, &buf[0], VertexArrayObject::kUsageStatic);
  */

  using Layout = AttributeLayoutPV<glm::vec3>;
  using LMesh = LayoutMesh<Layout>;
  using LMeshFilter = LayoutMeshFilter<Layout>;
  auto mf = a->AddComponent<LMeshFilter>();
  auto m = std::make_shared<LMesh>(3);
  //m->Set<0>( {
  //    glm::vec3(-1, -1, 0),
  //    glm::vec3( 1, -1, 0),
  //    glm::vec3( 0,  1, 0),
  //    } );
  m->Set<0>( 0, glm::vec3(-1,  -1,  0) );
  m->Set<0>( 1, glm::vec3( 1,  -1,  0) );
  m->Set<0>( 2, glm::vec3( 0,   1,  0) );
  mf->SetMesh(m, VertexArrayObject::kUsageStatic);

  //auto a = scene.Get<Actor>("actor.terrain");
  //a->AddComponent<MyMeshFilter>();
  //std::cerr << (a->GetComponent<MeshFilter>() == nullptr) << std::endl;
  //std::cerr << (a->GetComponent<MeshFilterBase>() == nullptr) << std::endl;
  //std::cerr << (a->GetComponent<MyMeshFilter>() == nullptr) << std::endl;

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
