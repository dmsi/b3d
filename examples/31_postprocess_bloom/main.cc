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

struct BypassUniform: public Action {
  Color     tint;
  bool      uv_flip_vert;
  bool      uv_flip_horiz;

  BypassUniform(std::shared_ptr<Transformation> transform, 
                const Color& a_tint   = Color(1),
                bool  a_uv_flip_vert  = false,
                bool  a_uv_flip_horiz = false)
    :  Action        (transform),
       tint          (a_tint),
       uv_flip_vert  (a_uv_flip_vert),
       uv_flip_horiz (a_uv_flip_horiz) {}

  void PreDraw() override {
    if (auto m = transform->GetActor().GetComponent<Material>()) {
      m->SetUniform("tint", tint);
      m->SetUniform("uv_flip_vert",  (int)uv_flip_vert);
      m->SetUniform("uv_flip_horiz", (int)uv_flip_horiz);
    }
  }
};

int main(int argc, char* argv[]) {
  using T     = std::vector<std::string>; 
  using TexLs = std::vector<std::shared_ptr<Texture>>;
  using glm::vec3;
  using glm::vec2;
  
  Scene scene;

  /////////////////////////////////////////////////////////////////////////////
  // Step 1. Initialize application.
  /////////////////////////////////////////////////////////////////////////////
  AppContext::Init(1280, 720, "Bloom [b3d]", Profile("3 3 core"));
  AppContext::Instance().display.ShowCursor(false);
  int width = AppContext::Instance().display.GetWidth();
  int height = AppContext::Instance().display.GetHeight();

  /////////////////////////////////////////////////////////////////////////////
  // Step 2. Setup render targets and post-processing pipeline
  /////////////////////////////////////////////////////////////////////////////
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

  // Downsample and highlight then blur, then downsample and blur again.
  // 5 passes in total, 11x11 gauss kernel.
  // Then take scene texture and combine with blur-highlighted. 
  int w = width / 8, h = height / 8;
  PostprocessPipeline pp_blur(scene, "assets/materials/postprocess");
  pp_blur.Input(10, TexLs{scene_tex});
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

  // Bloom combine textures
  w = width, h = height;
  PostprocessPipeline pp_bloom(scene, "assets/materials/postprocess");
  pp_bloom.Input(20, TexLs{scene_tex, pp_blur.Tex()}); 
  pp_bloom.Stage("bloom",  w,  h,  "bloom_combine.mat");
  pp_bloom.Done();

  // Get different stages outputs in order to show them on the screen
  auto hightilght_tex = pp_blur.Tex(0);
  auto blur_tex = pp_blur.Tex();
  auto final_tex = pp_bloom.Tex(); 
  
  /////////////////////////////////////////////////////////////////////////////
  // Step 3. 4x4 Displays. Transformations in NDC, aspect ratio not changed. 
  /////////////////////////////////////////////////////////////////////////////
  Cfg<Actor>(scene, "actor.display.lt")
    . Model("assets/models/screen.dsm", "assets/materials/overlay_texture.mat")
    . Tags(0, T{"pp-final"})
    . Texture(0, scene_tex)
    . Scale   ( .5,  .5,  1)
    . Position(-.5,  .5,  0)
    . Done();
  
  Cfg<Actor>(scene, "actor.display.rt")
    . Model("assets/models/screen.dsm", "assets/materials/overlay_texture.mat")
    . Tags(0, T{"pp-final"})
    . Texture(0, hightilght_tex)
    . Scale   ( .5,  .5,  1)
    . Position( .5,  .5,  0)
    . Done();
  
  Cfg<Actor>(scene, "actor.display.lb")
    . Model("assets/models/screen.dsm", "assets/materials/overlay_texture.mat")
    . Tags(0, T{"pp-final"})
    . Texture(0, blur_tex)
    . Scale   ( .5,  .5,  1)
    . Position(-.5, -.5,  0)
    . Done();

  Cfg<Actor>(scene, "actor.display.rb")
    . Model("assets/models/screen.dsm", "assets/materials/overlay_texture.mat")
    . Tags(0, T{"pp-final"})
    . Texture(0, final_tex)
    . Scale   ( .5,  .5,  1)
    . Position( .5, -.5,  0)
    . Done();
  
  /////////////////////////////////////////////////////////////////////////////
  // Step 3. Compose part of the scene for rendering to texture 
  /////////////////////////////////////////////////////////////////////////////
  Cfg<Camera>(scene, "camera.main")
    . Perspective(60, (float)width/height, .1, 150)
    . Position   (  0, 3, 3)
    . EulerAngles(-30, 0, 0)
    . Action<FlyingCameraController>(5)
    . Done();

  Cfg<Actor>(scene, "actor.skybox")
    . Model("assets/models/sphere.dsm", "assets/materials/skybox_cubemap.mat")
    . Done();

  Cfg<Actor>(scene, "actor.knight1")
    . Model("assets/models/knight.dsm", "assets/materials/texture.mat")
    . Position   (-2, 0, 0)
    . EulerAngles(0, 30, 0)
    . Done();
  
  Cfg<Actor>(scene, "actor.knight2")
    . Model("assets/models/knight.dsm", "assets/materials/texture.mat")
    . Action<Rotator>(vec3(0, 30, 0))
    . Done();
  
  Cfg<Actor>(scene, "actor.knight3")
    . Model("assets/models/knight.dsm", "assets/materials/texture.mat")
    . Position   (2, 0, 0)
    . EulerAngles(0, -30, 0)
    . Done();
  
  // FPS meter. Goes to the final screen.
  Cfg<Actor>(scene, "actor.fps.meter")
    . Action<FpsMeter>()
    . Tags(0, T{"pp-final"})
    . Done();
  
  /////////////////////////////////////////////////////////////////////////////
  // Step 5. Main loop. Press ESC to exit.
  /////////////////////////////////////////////////////////////////////////////
  do {
    AppContext::BeginFrame();
    scene.Update();
    scene.Draw();
    AppContext::EndFrame();
  } while (AppContext::Running());

  /////////////////////////////////////////////////////////////////////////////
  // Step 6. Cleanup and close the app.
  /////////////////////////////////////////////////////////////////////////////
  AppContext::Close();
  return 0;
}
