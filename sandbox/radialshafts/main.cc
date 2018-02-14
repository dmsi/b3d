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

void InitPostProcessingPipeline(Scene& scene) {
  using glm::vec2;
  using T     = std::vector<std::string>; 
  using TexLs = std::vector<std::shared_ptr<Texture>>;

  int width = AppContext::Instance().display.GetWidth();
  int height = AppContext::Instance().display.GetHeight();

  // Texture to render scene, input to postprocessing pipeline
  auto scene_tex = Cfg<RenderTarget>(scene, "rt.scene", 0) 
      . Tags("onscreen")
      . Type(FrameBuffer::kTexture2D)
      . Resolution(width, height)
      . Layer(Layer::kColor, Layer::kReadWrite)
      . Layer(Layer::kDepth, Layer::kWrite)
      . Clear(0, 0, 0, 1)
      . Done()
      ->GetLayerAsTexture(0, Layer::kColor);
  
  // hightlight/downscale -> radial blur -> gauss blur -> combine
  int w = width / 8, h = height / 8;
  PostprocessPipeline pp_highlight(scene, "Assets/Postprocess");
  pp_highlight.Input(10, TexLs{scene_tex});
  pp_highlight.Stage("highlight",   w,  h,  "bloom_highlight.mat");
  pp_highlight.Done();

  PostprocessPipeline pp_radblur(scene, "sandbox/radialshafts");
  pp_radblur.Input(20, TexLs{pp_highlight.Tex()});
  pp_radblur.Stage("radialblur", w, h, "radial_blur.mat");
  pp_radblur.Done();
  
  PostprocessPipeline pp_gaussblur(scene, "Assets/Postprocess");
  pp_gaussblur.Input(25, TexLs{pp_radblur.Tex()});
  pp_gaussblur
    . Stage("hblur",       w,  h,  "gauss_blur.mat")
    . Action<BlurUniform> (w,  h,  vec2(1, 0));
  pp_gaussblur
    . Stage("vblur",       w,  h,  "gauss_blur.mat")
    . Action<BlurUniform> (w,  h,  vec2(0, 1));
  w /= 2; h /= 2;
  pp_gaussblur
    . Stage("hblur2",      w,  h,  "gauss_blur.mat")
    . Action<BlurUniform> (w,  h,  vec2(1, 0));
  pp_gaussblur
    . Stage("vblur2",      w,  h,  "gauss_blur.mat")
    . Action<BlurUniform> (w,  h,  vec2(0, 1));
  pp_gaussblur.Done();
  
  PostprocessPipeline pp_final(scene, "Assets/Postprocess");
  pp_final.Input(30, TexLs{scene_tex, pp_gaussblur.Tex()});
  pp_final.Stage("combine",   width,  height,  "bloom_combine.mat");
  pp_final.Done();

  auto final_tex = pp_final.Tex(); 

  Cfg<Actor>(scene, "actor.display.final")
    . Model("Assets/screen.dsm", "Assets/overlay_texture.mat")
    . Tags(0, T{"pp-final"})
    . Texture(0, final_tex)
    . Done();
}

int main(int argc, char* argv[]) {
  using glm::vec3;
  using T     = std::vector<std::string>; 
  Scene scene;

  // Initialize application.
  AppContext::Init(1280, 720, "Radial light shafts [b3d]", Profile("4 0 core"));
  AppContext::Instance().display.ShowCursor(false);
  int width = AppContext::Instance().display.GetWidth();
  int height = AppContext::Instance().display.GetHeight();

  // Setup main and shadowmap rendertargets
  Cfg<RenderTarget>(scene, "rt.screen", 2000)
    . Tags("pp-final")
    . Clear(.0, .0, .0, 1)
    . Done();

  // To disable postprocessing pipeline, the following is required:
  //  1. Remove this function call
  //  2. Add onscreen tag to main render target 
  InitPostProcessingPipeline(scene);

  Cfg<Actor>(scene, "actor.knight")
    . Model("Assets/icosahedron.dsm", "sandbox/radialshafts/tesselation.mat")
    . Action<Rotator>(glm::vec3(0, 60, 60))
    . Done()
    ->GetComponent<MeshRenderer>()
    ->primitive = MeshRenderer::kPtPatches;

  // Main camera
  Cfg<Camera>(scene, "camera.main")
    . Perspective(60, (float)width/height, .1, 100)
    . Position(0, 0, 4)
    . Action<FlyingCameraController>(5)
    . Done();
  
  // Fps meter.
  Cfg<Actor>(scene, "actor.fps.meter")
    . Action<FpsMeter>()
    . Tags(0, T{"pp-final"})
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
