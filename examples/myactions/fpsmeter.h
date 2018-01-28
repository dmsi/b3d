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

#ifndef _ACTION_FPSMETER_H_3A8F1324_B798_446E_8F94_AC0F61693380_
#define _ACTION_FPSMETER_H_3A8F1324_B798_446E_8F94_AC0F61693380_ 

#include "action.h"
#include "textlabel.h"
#include "material/material_loader.h"

class FpsMeter : public Action {
 public:
  enum { kAverageCount = 120 };

  std::vector<std::string> tags;

  float      fps_buffer[kAverageCount];
  int        fps_frame;
  float      avg_fps;
  TextLabel  fps_label;
  BitmapFont fps_label_font;

  float      beauty_color; 
  int        beauty_direction;

  FpsMeter(std::shared_ptr<Transformation> transform,
           std::vector<std::string> new_tags = {}) 
    : Action(transform), 
      tags(std::move(new_tags)),
      fps_label(1.8f*1.2f * glm::vec2(1.0f,1.0f), glm::vec2(20.0f, 10.0f),    
               transform->GetActor().AddComponent<MeshFilter>(),
               transform->GetActor().AddComponent<MeshRenderer>()),
      fps_label_font("Assets/Fonts/mono.fnt") {
    for (float& f: fps_buffer) {
      f = 0;
    }
    avg_fps = 0;
    fps_frame = 0;
    fps_label.SetText(fps_label_font, "Hello!");
    if (auto mr = transform->GetActor().GetComponent<MeshRenderer>()) {
      mr->SetMaterial(MaterialLoader::Load("Assets/Fonts/mono.mat"));
      if (!tags.empty()) {
        mr->GetMaterial()->GetPass(0)->SetTags(tags);
      }
    }
    beauty_color = 0;
    beauty_direction = 1;
  }

  void Update() override {
    float delta_time = GetTimer().GetTimeDelta();
    if (delta_time == 0) {
      return;
    }

    fps_buffer[fps_frame] = 1.0f / delta_time;
    fps_frame = (fps_frame + 1) % kAverageCount;

    avg_fps = 0;
    for (int i = 0; i < kAverageCount; i++) {
      avg_fps += fps_buffer[i];
    }
    avg_fps /= kAverageCount;

    fps_label.SetText(fps_label_font, GetFps());
  }
  
  void PreDraw() override {
    if (auto mtrl = transform->GetActor().GetComponent<Material>()) {
      beauty_color += beauty_direction * GetTimer().GetTimeDelta(); 
      if (beauty_color > 1) {
        beauty_color = 1;
        beauty_direction = -1;
      } else if (beauty_color < 0) {
        beauty_color = 0;
        beauty_direction = 1;
      }
      mtrl->SetUniform("TextColor", glm::vec4(beauty_color,1-beauty_color,beauty_color/2,1));
    }
  }

  std::string GetFps() const {
    return std::to_string((int)avg_fps);
  }
};

#endif // _ACTION_FPSMETER_H_3A8F1324_B798_446E_8F94_AC0F61693380_
