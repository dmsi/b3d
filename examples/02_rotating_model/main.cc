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

// Rotating associated actor in all directions.
struct MyRotator: public Action {
  glm::vec3 rotation_speed;
  MyRotator(std::shared_ptr<Transformation> transform): Action(transform) {}

  void Update() override {
    transform->Rotate(rotation_speed * GetTimer().GetTimeDelta());
  }
};

int main(int argc, char* argv[]) {
  Scene scene;

  // Step 1. Initialize application.
  AppContext::Init(1280, 720, "Rotating 3D model [b3d]", Profile("3 3 core"));
  AppContext::Instance().display.ShowCursor(false);
  int width = AppContext::Instance().display.GetWidth();
  int height = AppContext::Instance().display.GetHeight();

  // Step 2. Prepare the actor. Using action in order to rotate.
  auto actor = scene.Add<Actor>("actor.knight");
  actor->AddComponent<MeshRenderer>()->SetMaterial(MaterialLoader::Load("Assets/texture.mat"));
  actor->AddComponent<MeshFilter>()->SetMesh(MeshLoader::Load("Assets/knight.dsm"));
  actor->AddAction<MyRotator>()->rotation_speed = glm::vec3(0, 45, 0);
  
  // Step 3. Define main camera. 
  std::shared_ptr<Camera> camera = scene.Add<Camera>("camera.main");
  camera->SetPerspective(60, 1.0f*width/height, 0.1, 150.0);
  camera->transform->SetLocalPosition(0, 3, 3);
  camera->transform->SetLocalEulerAngles(-30, 0, 0);
  
  // Step 4. Setup RenderTarget and FrameBuffer.
  Cfg<RenderTarget>(scene, "rt.screen", 2000)
    . Tags("onscreen")
    . Clear(.4, .4, .4, 1)
    . Done();

  // Step 5. Main loop. Press ESC to exit.
  do {
    AppContext::BeginFrame();
    scene.Update();
    scene.Draw();
    AppContext::EndFrame();
  } while (AppContext::Running());

  // Step 6. Cleanup and close the app.
  AppContext::Close();
  return 0;
}
