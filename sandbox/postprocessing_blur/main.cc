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
  Color     tint;
  bool      uv_flip_vert;
  bool      uv_flip_horiz;

  // C++ is a bit ugly here
  BlurUniform(std::shared_ptr<Transformation> transform, 
                           int       a_width,
                           int       a_height,
                           glm::vec2 a_direction, 
                           Color     a_tint          = Color(1),
                           bool      a_uv_flip_vert  = false,
                           bool      a_uv_flip_horiz = false)
    :  Action        (std::move(transform)),
       width         (a_width),
       height        (a_height),
       direction     (a_direction),
       tint          (std::move(a_tint)),
       uv_flip_vert  (a_uv_flip_vert),
       uv_flip_horiz (a_uv_flip_horiz) {}

  void PreDraw() override {
    if (auto m = transform->GetActor().GetComponent<Material>()) {
      m->SetUniform("tint", tint);
      m->SetUniform("uv_flip_vert",  (int)uv_flip_vert);
      m->SetUniform("uv_flip_horiz", (int)uv_flip_horiz);
      m->SetUniform("target_width",  width); 
      m->SetUniform("target_height", height); 
      m->SetUniform("direction",     direction); 
    }
  }
};

struct OverlayUniform: public Action {
  Color     tint;
  bool      uv_flip_vert;
  bool      uv_flip_horiz;

  OverlayUniform(std::shared_ptr<Transformation> transform, 
                           Color     a_tint          = Color(1),
                           bool      a_uv_flip_vert  = false,
                           bool      a_uv_flip_horiz = false)
    :  Action        (std::move(transform)),
       tint          (std::move(a_tint)),
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
  Scene scene;
  using T = std::vector<std::string>; // Tags list shortcut
  using glm::vec3;
  using glm::vec2;

  // Step 1. Initialize application.
  AppContext::Init(1280, 720, "DuDv distortion [b3d]", Profile("3 3 core"));
  AppContext::Instance().display.ShowCursor(false);
  int width = AppContext::Instance().display.GetWidth();
  int height = AppContext::Instance().display.GetHeight();

  // Step 2. Setup render targets. 
  Cfg<RenderTarget>(scene, "rt.screen", 2000)
    . Tags("pp-final")
    . Clear(.8, .4, .4, 1)
    . Done();
    
  int w = width, h = height; float d = 16;
  auto scene_tex = Cfg<RenderTarget>(scene, "rt.scene", 0) 
      . Tags("onscreen")
      . Type(FrameBuffer::kTexture2D)
      . Resolution(w, h)
      . Layer(Layer::kColor, Layer::kReadWrite)
      . Layer(Layer::kDepth, Layer::kWrite)
      . Clear(.4, .4, .4, 1)
      . Done()
      ->GetLayerAsTexture(0, Layer::kColor);

  d = 8; auto d2 = d * 2; auto d3 = d * 3;
  //PostprocessPipeline pp_blur(scene);
  //pp_blur.Input(10, PostprocessPipeline::TexLs{scene_tex});
  //pp_blur.Stage("pp-downscale", w/d,  h/d,  "Assets/overlay_texture.mat");
  //pp_blur.Stage("pp-hblur",     w/d,  h/d,  "Assets/pp_gauss_blur.mat");
  //pp_blur.Stage("pp-vblur",     w/d,  h/d,  "Assets/pp_gauss_blur.mat");
  //pp_blur.Stage("pp-hblur2",    w/d2, h/d2, "Assets/pp_gauss_blur.mat");
  //pp_blur.Stage("pp-vblur2",    w/d2, h/d2, "Assets/pp_gauss_blur.mat");
  //pp_blur.Stage("pp-hblur3",    w/d3, h/d3, "Assets/pp_gauss_blur.mat");
  //pp_blur.Stage("pp-vblur3",    w/d3, h/d3, "Assets/pp_gauss_blur.mat");

  //pp_blur.Action<BlurUniform>("pp-hblur",  w/d, h/d,   vec2(1, 0));
  //pp_blur.Action<BlurUniform>("pp-vblur",  w/d, h/d,   vec2(0, 1));
  //pp_blur.Action<BlurUniform>("pp-hblur2", w/d2, h/d2, vec2(1, 0));
  //pp_blur.Action<BlurUniform>("pp-vblur2", w/d2, h/d2, vec2(0, 1));
  //pp_blur.Action<BlurUniform>("pp-hblur3", w/d3, h/d3, vec2(1, 0));
  //pp_blur.Action<BlurUniform>("pp-vblur3", w/d3, h/d3, vec2(0, 1));
  //pp_blur.Done();

  PostprocessPipeline pp_blur(scene);
  pp_blur.Input(10, PostprocessPipeline::TexLs{scene_tex});
  pp_blur
    . Stage("downscale",   w/d,  h/d,    "Assets/overlay_texture.mat");
  pp_blur
    . Stage("hblur",       w/d,  h/d,    "Assets/pp_gauss_blur.mat")
    . Action<BlurUniform> (w/d,  h/d,    vec2(1, 0));
  pp_blur
    . Stage("vblur",       w/d,  h/d,    "Assets/pp_gauss_blur.mat")
    . Action<BlurUniform> (w/d,  h/d,    vec2(0, 1));
  pp_blur
    . Stage("hblur2",      w/d2,  h/d2,  "Assets/pp_gauss_blur.mat")
    . Action<BlurUniform> (w/d2,  h/d2,  vec2(1, 0));
  pp_blur
    . Stage("vblur2",      w/d2,  h/d2,  "Assets/pp_gauss_blur.mat")
    . Action<BlurUniform> (w/d2,  h/d2,  vec2(0, 1));
  pp_blur.Done();

  auto final_tex = pp_blur.Tex(); 
  
  
  /////////////////////////////////////////////////////////////////////////////
  // Step 4. 4x4 Displays. Transformations in NDC, aspect ratio not changed. 
  /////////////////////////////////////////////////////////////////////////////
  Cfg<Actor>(scene, "actor.display.lt")
    . Model("Assets/screen.dsm", "Assets/overlay_texture.mat")
    . Tags(0, T{"pp-final"})
    . Texture(0, scene_tex)
    . Scale   ( .5,  .5,  1)
    . Position(-.5,  .5,  0)
    . Done();
  
  Cfg<Actor>(scene, "actor.display.rt")
    . Model("Assets/screen.dsm", "Assets/overlay_texture.mat")
    . Tags(0, T{"pp-final"})
    . Texture(0, final_tex)
    . Scale   ( .5,  .5,  1)
    . Position( .5,  .5,  0)
    . Done();
  
  //Cfg<Actor>(scene, "actor.display.lb")
  //  . Model("Assets/screen.dsm", "Assets/overlay_texture.mat")
  //  . Tags(0, T{"post-processing"})
  //  . Texture(0, scene_tex)
  //  . Scale   ( .5,  .5,  1)
  //  . Position(-.5, -.5,  0)
  //  . Done();

  //Cfg<Actor>(scene, "actor.display.rb")
  //  . Model("Assets/screen.dsm", "Assets/overlay_texture.mat")
  //  . Tags(0, T{"post-processing"})
  //  . Texture(0, scene_tex)
  //  . Scale   ( .5,  .5,  1)
  //  . Position( .5, -.5,  0)
  //  . Done();
  
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
    . Model("Assets/sphere.dsm", "Assets/skybox_cubemap.mat")
    . Done();

  Cfg<Actor>(scene, "actor.knight1")
    . Model("Assets/knight.dsm", "Assets/texture.mat")
    . Position   (-2, 0, 0)
    . EulerAngles(0, 30, 0)
    . Done();
  
  Cfg<Actor>(scene, "actor.knight2")
    . Model("Assets/knight.dsm", "Assets/texture.mat")
    . Action<Rotator>(vec3(0, 30, 0))
    . Done();
  
  Cfg<Actor>(scene, "actor.knight3")
    . Model("Assets/knight.dsm", "Assets/texture.mat")
    . Position   (2, 0, 0)
    . EulerAngles(0, -30, 0)
    . Done();
  
  /////////////////////////////////////////////////////////////////////////////
  // FPS meter. Goes to the final screen.
  /////////////////////////////////////////////////////////////////////////////
  Cfg<Actor>(scene, "actor.fps.meter")
    . Action<FpsMeter>()
    . Tags(0, T{"pp-final"})
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
