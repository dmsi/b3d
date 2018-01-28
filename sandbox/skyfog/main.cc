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

struct BlurUniform: public Action {
  int       width;
  int       height;
  glm::vec2 direction;

  BlurUniform(std::shared_ptr<Transformation> transform, 
              int a_width, int a_height, glm::vec2 a_direction)
    :  Action(transform), width(a_width), height(a_height), 
       direction(a_direction) {}

  void PreDraw() override {
    if (auto m = transform->GetActor().GetComponent<Material>()) {
      m->SetUniform("target_width",  width); 
      m->SetUniform("target_height", height); 
      m->SetUniform("direction",     direction); 
    }
  }
};


int main(int argc, char* argv[]) {
  Scene scene;
  using TexLs = std::vector<std::shared_ptr<Texture>>;
  using T = std::vector<std::string>;
  using glm::vec2;

  // Step 1. Initialize application.
  AppContext::Init(1280, 720, "Skybox cubemap [b3d]", Profile("3 3 core"));
  AppContext::Instance().display.ShowCursor(false);
  int width = AppContext::Instance().display.GetWidth();
  int height = AppContext::Instance().display.GetHeight();
  
  // Step 2. Setup RenderTarget.
  Cfg<RenderTarget>(scene, "rt.screen", 2000)
    . Tags("pp-final")
    . Clear(.8, .4, .4, 1)
    . Done();
    
  auto scene_tex = Cfg<RenderTarget>(scene, "rt.scene", 0) 
      . Tags("onscreen")
      . Type(FrameBuffer::kTexture2D)
      . Resolution(width, height)
      . Layer(Layer::kColor, Layer::kReadWrite)
      . Layer(Layer::kDepth, Layer::kWrite)
      . Clear(.4, .4, .4, 1)
      . Done()
      ->GetLayerAsTexture(0, Layer::kColor);
  
  // Downsample then blur, then downsample and blur again
  // 5 passes in total, 11x11 gauss kernel.
  int w = width / 8, h = height / 8;
  PostprocessPipeline pp_blur(scene, "Assets/Postprocess");
  pp_blur.Input(10, PostprocessPipeline::TexLs{scene_tex});
  pp_blur
    . Stage("highlight",   w,  h,  "bloom_highlight.mat");
  pp_blur
    . Stage("hblur",       w,  h,  "gauss_blur.mat")
    . Action<BlurUniform> (w,  h,  vec2(1, 0));
  pp_blur
    . Stage("vblur",       w,  h,  "gauss_blur.mat")
    . Action<BlurUniform> (w,  h,  vec2(0, 1));
  w /= 2; h /= 2;
  pp_blur
    . Stage("hblur2",      w,  h,  "gauss_blur.mat")
    . Action<BlurUniform> (w,  h,  vec2(1, 0));
  pp_blur
    . Stage("vblur2",      w,  h,  "gauss_blur.mat")
    . Action<BlurUniform> (w,  h,  vec2(0, 1));
  pp_blur.Done();

  w = width, h = height;
  PostprocessPipeline pp_bloom(scene, "Assets/Postprocess");
  pp_bloom.Input(20, TexLs{scene_tex, pp_blur.Tex()}); 
  pp_bloom.Stage("bloom",  w,  h,  "bloom_combine.mat");
  pp_bloom.Done();
  auto final_tex = pp_bloom.Tex(); 
  
  Cfg<Actor>(scene, "actor.display.lt")
    . Model("Assets/screen.dsm", "Assets/overlay_texture.mat")
    . Tags(0, T{"pp-final"})
    . Texture(0, final_tex)
    . Done();

  // Step 3. Compose the scene. 
  Cfg<Camera>(scene, "camera.main")
    . Perspective(60, (float)width/height, 1, 1500)
    . Position(0, 50, 0)
    . Action<FlyingCameraController>(150)
    . Done();
  
  Cfg<Light>(scene, "light.sun")
    . Position(0, 5, 0)
    . EulerAngles(70, -30, -20)
    . Color(1, 1, 1, 1) 
    . Done();

  Cfg<Actor>(scene, "actor.terrain")
    . Model("Assets/terrain.dsm", "Assets/terrain_flatshading_fog_v2.mat")
    . Done();

  Cfg<Actor>(scene, "actor.skybox")
    . Model("Assets/blender_cube.dsm", "Assets/skybox_fog_cubemap_v2.mat")
    . Done();

  Cfg<Actor>(scene, "actor.fps.meter")
    . Action<FpsMeter>()
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
