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

struct SetColorUniform: public Action {
  glm::vec4 color;

  SetColorUniform(std::shared_ptr<Transformation> transform) 
    : Action(transform), color(1, 1, 1, 1) {}

  void PreDraw() override {
    if (auto mtrl = transform->GetActor().GetComponent<Material>()) {
      mtrl->SetUniform("Color", color);
    }
  }
};

// Counter clockwise quad with UV set
std::shared_ptr<Mesh> MakeQuadMesh(const glm::vec3& offset = glm::vec3(0,0,0)) {
  std::shared_ptr<Mesh> mesh(new Mesh());
  mesh->vertices = { 
    glm::vec3(0, 0, 0), // 0 - bottom left
    glm::vec3(1, 0, 0), // 1 - bottom right
    glm::vec3(1, 1, 0), // 2 - top right
    glm::vec3(0, 1, 0)  // 3 - top left
  }; 

  mesh->uv = { 
    glm::vec2(0, 0),    // 0 - bottom left
    glm::vec2(1, 0),    // 1 - bottom right
    glm::vec2(1, 1),    // 2 - top right
    glm::vec2(0, 1)     // 3 - top left
  }; 

  mesh->indices = {
    0, 1, 2, 0, 2, 3    // CCW
  };  
  
  float margin = 0.05f;
  for (auto& v: mesh->vertices) {
    v *= (1.0f - margin); 
    v += (offset + glm::vec3(margin/2, margin/2, 0));
  }

  return mesh;
}

int main(int argc, char* argv[]) {
  Scene scene;

  // Step 1. Initialize application.
  AppContext::Init(1280, 720, "Rendering to texture [b3d]", Profile("3 3 core"));
  AppContext::Instance().display.ShowCursor(false);
  int width = AppContext::Instance().display.GetWidth();
  int height = AppContext::Instance().display.GetHeight();

  // Step 2. Setup offscreen RenderTarget and FrameBuffer.
  // The magic number 0 here is the order in which this render target
  // will be rendered in relation to other render targets. Less numbers 
  // go first. We want offscreen target to be rendered first because we are
  // going to use the textures it generates - color and depth.
  // The tags parameter is responsible for filtering Meterial passes
  // by queue. If any of material pass tags matches with any of RT tags 
  // it will be accepted by the render target and  discarded otherwise. 
  // Render target rebuilds list of passes every frame. 
  auto fb_offscreen = Cfg<RenderTarget>(scene, "rt.offscreen", 0) 
    . Tags("offscreen")
    . Type(FrameBuffer::kTexture2D)
    . Resolution(100, 100) // nice downscaling pixelization
    . Layer(Layer::kColor, Layer::kReadWrite, Texture::kFilterPoint)
    . Layer(Layer::kDepth, Layer::kReadWrite, Texture::kFilterPoint)
    . Clear(.4, .4, .4, 1)
    . Done();

  // Step 2.1. Retrieve the textures where the offscreen render target will
  // render its content. It must be called after FrameBuffer::Init().
  auto color_tex = fb_offscreen->GetLayerAsTexture(0, Layer::kColor);
  auto depth_tex = fb_offscreen->GetLayerAsTexture(0, Layer::kDepth);

  // Step 2.2. Setup the model which is going to be rendered to texture.
  auto actor = scene.Add<Actor>("actor.knight");
  actor->AddComponent<MeshRenderer>()->SetMaterial(MaterialLoader::Load("Assets/Materials/texture.mat"));
  actor->AddComponent<MeshFilter>()->SetMesh(MeshLoader::Load("Assets/knight.dsm"));
  actor->AddAction<Rotator>()->rotation_speed = glm::vec3(0, 45, 0);
  actor->GetComponent<MeshRenderer>()->GetMaterial()->GetPass(0)->SetTags({"offscreen"});
  
  // Step 3. Setup onscreen RenderTarget and FrameBuffer.
  // Use 1 for order to make sure it renders after the offscreen.
  // tag=onscreen we want for the objects directly rendered to screen.
  //
  // We are going to use 4 "screens", to show the rendered to texture model with
  // different color filters. These 4 screens will cover 100% of the window, so
  // we dont need to clear the screen color layer.
  // 
  // We are not using depth, so no specifying it here, also FrameBuffer::Init is 
  // not needed for the screen render target, so we omit it in this example.
  Cfg<RenderTarget>(scene, "rt.screen", 1)
    . Tags("onscreen")
    . Clear(.4, .4, .4, 1)
    . Done();
 
  // Step 3.1. Create 4 "screens" 
  // Right Top - Red filter
  auto screen_rt = scene.Add<Actor>("actor.screen_right_top");
  screen_rt->AddComponent<MeshRenderer>()->SetMaterial(MaterialLoader::Load("Assets/Materials/Core/screen_color_r2t.mat"));
  screen_rt->AddComponent<MeshFilter>()->SetMesh(MakeQuadMesh());
  screen_rt->AddAction<SetColorUniform>()->color = glm::vec4(1, 0, 0, 1);
  // Set color texture from the offscreen render target 
  screen_rt->GetComponent<MeshRenderer>()->GetMaterial()->SetTexture(0, color_tex);
  
  // Left Top - Green filter
  auto screen_lt = scene.Add<Actor>("actor.screen_left_top");
  screen_lt->AddComponent<MeshRenderer>()->SetMaterial(MaterialLoader::Load("Assets/Materials/Core/screen_color_r2t.mat"));
  screen_lt->AddComponent<MeshFilter>()->SetMesh(MakeQuadMesh(glm::vec3(-1, 0, 0)));
  screen_lt->AddAction<SetColorUniform>()->color = glm::vec4(0, 1, 0, 1);
  // Set color texture from the offscreen render target 
  screen_lt->GetComponent<MeshRenderer>()->GetMaterial()->SetTexture(0, color_tex);

  // Left Bottom - Normal filter
  auto screen_lb = scene.Add<Actor>("actor.screen_left_bottom");
  screen_lb->AddComponent<MeshRenderer>()->SetMaterial(MaterialLoader::Load("Assets/Materials/Core/screen_color_r2t.mat"));
  screen_lb->AddComponent<MeshFilter>()->SetMesh(MakeQuadMesh(glm::vec3(-1, -1, 0)));
  screen_lb->AddAction<SetColorUniform>()->color = glm::vec4(1, 1, 1, 1);
  // Set color texture from the offscreen render target 
  screen_lb->GetComponent<MeshRenderer>()->GetMaterial()->SetTexture(0, color_tex);
  
  // Right Bottom - Depth texture 
  auto screen_rb = scene.Add<Actor>("actor.screen_right_bottom");
  screen_rb->AddComponent<MeshRenderer>()->SetMaterial(MaterialLoader::Load("Assets/Materials/Core/screen_depth_r2t.mat"));
  screen_rb->AddComponent<MeshFilter>()->SetMesh(MakeQuadMesh(glm::vec3(0, -1, 0)));
  // Set depth texture from the offscreen render target 
  screen_rb->GetComponent<MeshRenderer>()->GetMaterial()->SetTexture(0, depth_tex);
  
  // Step 3.2. Add FPS meter.
  // Set Pass/Queue to 2000 in order to draw this after the 'screens'. 
  // Another way to draw objects one on top of another is to use Z component
  // of screen/fps coordinates. We are not using transformations in the shader,
  // but the depth buffer could handle this anyway.
  scene.Add<Actor>("actor.fps_meter")->AddAction<FpsMeter>();

  // Step 4. Set main camera. 
  std::shared_ptr<Camera> camera = scene.Add<Camera>("camera.main");
  camera->SetPerspective(60, 1.0f*width/height, 0.1, 150.0);
  camera->transform->SetLocalPosition(glm::vec3(0, 3, 3));
  camera->transform->SetLocalEulerAngles(glm::vec3(-30, 0, 0));

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
