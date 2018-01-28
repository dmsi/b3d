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

#ifndef _UNIFORMSTORAGE_H_E2F3462F_43CD_4694_9DFE_B3B3E92CA120_
#define _UNIFORMSTORAGE_H_E2F3462F_43CD_4694_9DFE_B3B3E92CA120_ 

#include <string>
#include <iostream>
#include <map>
#include "glenum.h"
#include "glm/glm.hpp"
#include <glm/gtc/type_precision.hpp>

#if 0
void Pass::SetUniforms(UniformStorage& storage) {
  for (it = shader->uniform_begin(); it != uniform_end(); ++it) {
    const Shader::UniformInfo& info = *it;
    GLenum type;
    void* uniform_value = storage.Get(info.name, type);
    if (type != info.type) {
      throw std::logic_error("Pass::SetUniforms() - Bad type");
    }
    shader->BindUniform(info.location, uniform_value, type);
  }
}

void Shader::SetUniform(const std::string& name, void* uniform, size_t size, GLenum type) {
  switch (type) {
    case GL_FLOAT      : glUniform1f(location, value);
    case GL_FLOAT_MAT4 : glUniformMatrix4fv(location, 1, GL_FALSE, uniform);

  }
}
// broadcast from the Material???
#endif

class ShaderUniformVariable;
namespace Shader_Uniform_cpp_cumbersome {
template <typename T>
void SetUniformValue(ShaderUniformVariable& variable, const T& value);
}

//////////////////////////////////////////////////////////////////////////////
// Placeholder to save uniform variable in order to pass later into the shader 
//////////////////////////////////////////////////////////////////////////////
class ShaderUniformVariable {
 public:
  ShaderUniformVariable() : type_((GLenum)-1), is_pointer_(false) {}

  template <typename T>
  explicit ShaderUniformVariable(const T& value) {
    Set(value);
  }
  
  template <typename T>
  void Set(const T& value) {
    Shader_Uniform_cpp_cumbersome::SetUniformValue(*this, value);
  }

  void* Get() {
    if (is_pointer_) {
      return value_.ptr;
    } else {
      return &value_.raw;
    }
  }

  GLenum GetType() const {
    return type_;
  }

 public:
  union {
    uint8_t            raw;
    void*              ptr;

    float              float1;
    float              float2[2];
    float              float3[3];
    float              float4[4];

    float              matrix_float_4x4[16];

    int                int1;

    ShaderUniformVariable*    value_self_ptr;
  } value_;

  GLenum type_;
  bool   is_pointer_;
};

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
class ShaderUniformStorage {
 public:
  template <typename T>
  void Set(const std::string& name, const T& value) {
    ShaderUniformVariable variable;
    variable.Set(value);
    the_storage_.emplace(name, variable);
  }

  void* Get(const std::string& name, GLenum& out_type) {
    auto it = the_storage_.find(name);
    if (it != the_storage_.end()) {
      out_type = it->second.GetType();
      return it->second.Get();
    } else {
      return nullptr;
    }
  }
 private:
  std::map<std::string, ShaderUniformVariable> the_storage_;
};

#include "uniformstorage.inl"

#endif // _UNIFORMSTORAGE_H_E2F3462F_43CD_4694_9DFE_B3B3E92CA120_
