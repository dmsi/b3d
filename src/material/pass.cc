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
    default: 
      throw std::runtime_error(
          "BlendFactor::ToOpenGL() - invalid blend factor " + std::to_string(v));
  }
}

int BlendOp::ToOpenGL(BlendOp::Value v) {
  switch (v) {
    case BlendOp::kAdd                   : return GL_FUNC_ADD;
    case BlendOp::kSub                   : return GL_FUNC_SUBTRACT;
    case BlendOp::kRevSub                : return GL_FUNC_REVERSE_SUBTRACT;
    default: 
      throw std::runtime_error(
          "BlendOp::ToOpenGL() - invalid blend op " + std::to_string(v));
  }
}

int CullMode::ToOpenGL(CullMode::Value v) {
  switch (v) {
    case CullMode::kCw                   : return GL_CW;
    case CullMode::kCcw                  : return GL_CCW;
    default: throw 
      std::runtime_error(
          "CullMode::ToOpenGL() - invalid mode " + std::to_string(v));
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
}

//////////////////////////////////////////////////////////////////////////////
// Render pass 
//////////////////////////////////////////////////////////////////////////////
void Pass::Bind() {
  if (!shader_) {
    throw std::logic_error("Pass::Bind() - shader not set!");
  }
  shader_->Bind();
  options.Bind();
}

void Pass::Unbind() {
  if (!shader_) {
    throw std::logic_error("Pass::Unbind() - shader not set!");
  }
  shader_->Unbind();
  options.Unbind();
}
