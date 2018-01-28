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

#ifndef _LIGHTINGSHADERUNIFORM_H_955754FA_32A4_4780_8DAE_8FE2A898F346_
#define _LIGHTINGSHADERUNIFORM_H_955754FA_32A4_4780_8DAE_8FE2A898F346_ 

#include "glm_main.h"
#include "action.h"

struct LightingShaderUniform: public Action {
  glm::vec4 diffuse;
  glm::vec4 ambient;
  glm::vec4 specular;
  float     shininess;

  LightingShaderUniform(std::shared_ptr<Transformation> transform) 
    : Action(transform), 
      diffuse(.5, .5, .5, 1),
      ambient(.1, .1, .1, 1),
      specular(1, 1, 1, 1),
      shininess(20) {}

  void PreDraw() override {
    if (auto mtrl = transform->GetActor().GetComponent<Material>()) {
      mtrl->SetUniform("surface.diffuse",   diffuse);
      mtrl->SetUniform("surface.ambient",   ambient);
      mtrl->SetUniform("surface.specular",  specular);
      mtrl->SetUniform("surface.shininess", shininess);
    }
  }

  void Setup(const glm::vec4& diff, 
             const glm::vec4& amb, 
             const glm::vec4& spec,
             float shine) {
    diffuse = diff;
    ambient = amb;
    specular = spec;
    shininess = shine;
  }
};

#endif // _LIGHTINGSHADERUNIFORM_H_955754FA_32A4_4780_8DAE_8FE2A898F346_
