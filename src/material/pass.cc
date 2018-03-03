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

#include "material/pass.h"
#include "gl_main.h"
#include <exception>

//////////////////////////////////////////////////////////////////////////////
// PassOptions
//////////////////////////////////////////////////////////////////////////////

int BlendFactor::ToOpenGL(BlendFactor::Value v) {
  switch (v) {
    case BlendFactor::kZero              : return GL_ZERO;
    case BlendFactor::kOne               : return GL_ONE;
    case BlendFactor::kSrcColor          : return GL_SRC_COLOR;
    case BlendFactor::kOneMinusSrcColor  : return GL_ONE_MINUS_SRC_COLOR;
    case BlendFactor::kDstColor          : return GL_DST_COLOR;
    case BlendFactor::kOneMinusDstColor  : return GL_ONE_MINUS_DST_COLOR;
    case BlendFactor::kSrcAlpha          : return GL_SRC_ALPHA;
    case BlendFactor::kOneMinusSrcAlpha  : return GL_ONE_MINUS_SRC_ALPHA;
    case BlendFactor::kDstAlpha          : return GL_DST_ALPHA;
    case BlendFactor::kOneMinusDstAlpha  : return GL_ONE_MINUS_DST_ALPHA;
    default: ABORT_F("Invalid blend factor %d", v);
  }
}

int BlendOp::ToOpenGL(BlendOp::Value v) {
  switch (v) {
    case BlendOp::kAdd                   : return GL_FUNC_ADD;
    case BlendOp::kSub                   : return GL_FUNC_SUBTRACT;
    case BlendOp::kRevSub                : return GL_FUNC_REVERSE_SUBTRACT;
    default: ABORT_F("Invalid blend op %d", v);
  }
}

int CullMode::ToOpenGL(CullMode::Value v) {
  switch (v) {
    case CullMode::kCw                   : return GL_CW;
    case CullMode::kCcw                  : return GL_CCW;
    default: ABORT_F("Invalid mode %d", v);
  }
}

int FillMode::ToOpenGL(FillMode::Value v) {
  switch (v) {
    case FillMode::kSolid                   : return GL_FILL;
    case FillMode::kWireframe               : return GL_LINE;
    case FillMode::kPoint                   : return GL_POINT;
    default: ABORT_F("Invalid mode %d", v);
  }
}

void PassOptions::Bind() {
  if (mask_.test(kZwrite)) {
    glDepthMask(zwrite_ ? GL_TRUE : GL_FALSE);
  }

  if (mask_.test(kZtest)) {
    if (ztest_) {
      glEnable(GL_DEPTH_TEST);
    } else {
      glDisable(GL_DEPTH_TEST);
    }
  }

  if (mask_.test(kCull)) {
    if (cull_ == CullMode::kOff) {
      glDisable(GL_CULL_FACE);
    } else {
      glEnable(GL_CULL_FACE);
      glCullFace(GL_BACK);
      glFrontFace(CullMode::ToOpenGL(cull_));
    }
  }

  if (mask_.test(kBlend)) {
      glEnable(GL_BLEND);
      glBlendFunc(BlendFactor::ToOpenGL(src_blend_factor_),
                  BlendFactor::ToOpenGL(dst_blend_factor_));
      glBlendEquation(BlendOp::ToOpenGL(blend_op_));
  }

  if (mask_.test(kClipping)) {
    for (const auto& i: clipping_planes_) {
      glEnable(GL_CLIP_DISTANCE0 + i);
    }
  }

  if (mask_.test(kFill)) {
    glPolygonMode(GL_FRONT_AND_BACK, FillMode::ToOpenGL(fill_)); 
  }
}

void PassOptions::Unbind() {
  if (mask_.test(kZwrite)) {
    if (!zwrite_) {
      glDepthMask(GL_TRUE);
    }
  }

  if (mask_.test(kZtest)) {
    if (!ztest_) {
      glEnable(GL_DEPTH_TEST);
    }
  }
  
  if (mask_.test(kCull)) {
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);
  }
  if (mask_.test(kBlend)) {
    glDisable(GL_BLEND);
  }
  
  if (mask_.test(kClipping)) {
    for (const auto& i: clipping_planes_) {
      glDisable(GL_CLIP_DISTANCE0 + i);
    }
  }
  
  if (mask_.test(kFill)) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); 
  }
}

//////////////////////////////////////////////////////////////////////////////
// Render pass 
//////////////////////////////////////////////////////////////////////////////
void Pass::Bind() {
  if (!shader_) {
    ABORT_F("Shader not set");
  }
  shader_->Bind();
  options.Bind();
}

void Pass::Unbind() {
  if (!shader_) {
    ABORT_F("Shader not set");
  }
  shader_->Unbind();
  options.Unbind();
}

void Pass::SetShader(std::shared_ptr<Shader> shader) {
  shader_ = shader;

  su_pvm_location_ = shader->GetUniformLocation("SU_PVM_MATRIX");
  su_p_location_ = shader->GetUniformLocation("SU_P_MATRIX");
  su_v_location_ = shader->GetUniformLocation("SU_V_MATRIX");
  su_m_location_ = shader->GetUniformLocation("SU_M_MATRIX");
  su_dirlight_dir_ = shader->GetUniformLocation("SU_DIRECTIONAL_LIGHT_DIRECTION_0");
  su_dirlight_col_ = shader->GetUniformLocation("SU_DIRECTIONAL_LIGHT_COLOR_0");
  for (int i = 0; i < sizeof(su_textures)/sizeof(int); ++i) {
    su_textures[i] = shader->GetUniformLocation("TEXTURE_" + std::to_string(i));
  }
  su_time_location_ = shader->GetUniformLocation("SU_TIME");
}
  
void Pass::SuPvmMatrix(const glm::mat4& pvm) {
  shader_->SetUniform(su_pvm_location_, pvm);
}

void Pass::SuPMatrix(const glm::mat4& p) {
  shader_->SetUniform(su_p_location_, p);
}

void Pass::SuVMatrix(const glm::mat4& v) {
  shader_->SetUniform(su_v_location_, v);
}

void Pass::SuMMatrix(const glm::mat4& m) {
  shader_->SetUniform(su_m_location_, m);
}

void Pass::SuDirLight(const glm::vec3& dir, const Color& col) {
  shader_->SetUniform(su_dirlight_dir_, dir);
  shader_->SetUniform(su_dirlight_col_, col);
}

void Pass::SuTextures() {
  for (int i = 0; i < sizeof(su_textures)/sizeof(int); ++i) {
    shader_->SetUniform(su_textures[i], i); 
  }
}

void Pass::SuTime(float time) {
  shader_->SetUniform(su_time_location_, time);
}
