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
  Scene scene;

  // Step 1. Initialize application
  AppContext::Init(1280, 720, "Hello World! [b3d]", Profile("3 3 core"));
  AppContext::Instance().display.ShowCursor(false);

  // Step 2. Prepare the triangle
  std::shared_ptr<Mesh> mesh(new Mesh);
  mesh->vertices = { 
    glm::vec3(-1, -1,  0), 
    glm::vec3( 1, -1,  0),
    glm::vec3( 0,  1,  0) 
  }; 

  auto triangle = scene.Add<Actor>("actor.triangle");
  triangle->AddComponent<MeshRenderer>()->SetMaterial(MaterialLoader::Load("Assets/color.mat"));
  triangle->AddComponent<MeshFilter>()->SetMesh(mesh);
  
  // Step 3. Define main camera. 
  // We are not using any P-V-M transformations in this tutorial, so no need 
  // to setup the camera position & orientation.
  // Still it is required to add at least camera.main.
  scene.Add<Camera>("camera.main");
  
  // Step 4. Setup RenderTarget and FrameBuffer
  Cfg<RenderTarget>(scene, "rt.screen")
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
