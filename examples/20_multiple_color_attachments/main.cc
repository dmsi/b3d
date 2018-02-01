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
  using T = std::vector<std::string>; // Tags list shortcut

  // Step 1. Initialize application.
  AppContext::Init(1280, 720, "Multiple color attachments [b3d]", Profile("3 3 core"));
  AppContext::Instance().display.ShowCursor(false);
  int width = AppContext::Instance().display.GetWidth();
  int height = AppContext::Instance().display.GetHeight();
  
  // Step 2. Setup render targets. 
  Cfg<RenderTarget>(scene, "rt.screen", 1)
    . Tags("post-processing")
    . Clear(.4, .4, .4, 1)
    . Done();

  // multiple color attachments RT
  auto mca_rt = Cfg<RenderTarget>(scene, "rt.offscreen", 0)
    . Tags("onscreen")
    . Type(FrameBuffer::kTexture2D)
    . Resolution(width, height)
    . Layer(Layer::kColor, Layer::kReadWrite)
    . Layer(Layer::kColor, Layer::kReadWrite)
    . Layer(Layer::kColor, Layer::kReadWrite)
    . Layer(Layer::kColor, Layer::kReadWrite)
    . Layer(Layer::kDepth, Layer::kWrite)
    . Clear(.8, .4, .4, 1)
    . Done();

  // Get texture placeholders, each layer corresponds to the layout(location=...)
  // output of fragment shader.
  auto tex0 = mca_rt->GetLayerAsTexture(0, Layer::kColor);
  auto tex1 = mca_rt->GetLayerAsTexture(1, Layer::kColor);
  auto tex2 = mca_rt->GetLayerAsTexture(2, Layer::kColor);
  auto tex3 = mca_rt->GetLayerAsTexture(3, Layer::kColor);
  
  // Step 3. Compose the scene 
  Cfg<Camera>(scene, "camera.main")
    . Perspective(60, (float)width/height, .1, 150)
    . Position   (  0, 4, 6)
    . EulerAngles(-30, 0, 0)
    . Action<FlyingCameraController>(5)
    . Done();

  Cfg<Actor>(scene, "actor.knight1")
    . Model("Assets/knight.dsm", "Assets/texture_mca.mat")
    . Position   (-2,  0, 0)
    . EulerAngles( 0, 30, 0)
    . Done();
  
  Cfg<Actor>(scene, "actor.knight2")
    . Model("Assets/knight.dsm", "Assets/texture_mca.mat")
    . Action<Rotator>(glm::vec3(0, 30, 0))
    . Done();
  
  Cfg<Actor>(scene, "actor.knight3")
    . Model("Assets/knight.dsm", "Assets/texture_mca.mat")
    . Position   (2,   0, 0)
    . EulerAngles(0, -30, 0)
    . Done();
  
  // 4 displays - all positioning in NDC
  Cfg<Actor>(scene, "actor.display.lt")
    . Model("Assets/screen.dsm", "Assets/overlay_texture.mat")
    . Tags(0, T{"post-processing"})
    . Texture(0, tex0)
    . Scale   ( .5,  .5,  1)
    . Position(-.5,  .5,  0)
    . Done();
  
  Cfg<Actor>(scene, "actor.display.rt")
    . Model("Assets/screen.dsm", "Assets/overlay_texture.mat")
    . Tags(0, T{"post-processing"})
    . Texture(0, tex1)
    . Scale   ( .5,  .5,  1)
    . Position( .5,  .5,  0)
    . Done();

  Cfg<Actor>(scene, "actor.display.lb")
    . Model("Assets/screen.dsm", "Assets/overlay_texture.mat")
    . Tags(0, T{"post-processing"})
    . Texture(0, tex2)
    . Scale   ( .5,  .5,  1)
    . Position(-.5, -.5,  0)
    . Done();

  Cfg<Actor>(scene, "actor.display.rb")
    . Model("Assets/screen.dsm", "Assets/overlay_texture.mat")
    . Tags(0, T{"post-processing"})
    . Texture(0, tex3)
    . Scale   ( .5,  .5,  1)
    . Position( .5, -.5,  0)
    . Done();
  
  Cfg<Actor>(scene, "actor.fps.meter")
    . Action<FpsMeter>()  
    . Tags(0, T{"post-processing"})
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
