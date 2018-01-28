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

namespace Shader_Uniform_cpp_cumbersome {

template <typename T> 
void SetUniformValue(ShaderUniformVariable& variable, const T& value) {
  std::cerr << "SetUniformValue - generic" << std::endl;
}

template <>
void SetUniformValue(ShaderUniformVariable& variable, const float& value) {
  variable.value_.float1 = value;
  variable.type_ = GL_FLOAT;
  std::cerr << "SetUniformValue - float: " << variable.value_.float1 << std::endl;
}

template <>
void SetUniformValue(ShaderUniformVariable& variable, const glm::vec2& value) {
  variable.value_.float2[0] = value.x;
  variable.value_.float2[1] = value.y;
  std::cerr << "SetUniformValue - float2: (" 
            << variable.value_.float2[0] << ", " 
            << variable.value_.float2[1] << ")" << std::endl;
  variable.type_ = GL_FLOAT_VEC2;
}

template <>
void SetUniformValue(ShaderUniformVariable& variable, const glm::vec3& value) {
  variable.value_.float3[0] = value.x;
  variable.value_.float3[1] = value.y;
  variable.value_.float3[2] = value.z;
  std::cerr << "SetUniformValue - float3: (" 
            << variable.value_.float3[0] << ", " 
            << variable.value_.float3[1] << ", " 
            << variable.value_.float3[2] << ")" << std::endl;
  variable.type_ = GL_FLOAT_VEC3;
}

template <>
void SetUniformValue(ShaderUniformVariable& variable, const glm::mat4& value) {
  const char* begin = (char*)&value[0][0];
  const char* end = begin + sizeof(value);
  char      * dest = (char*)variable.value_.matrix_float_4x4;

  std::copy(begin, end, dest);
  std::cerr << "SetUniformValue - mat4x4: " 
            << "(" 
            << variable.value_.matrix_float_4x4[0] << ", " 
            << variable.value_.matrix_float_4x4[1] << ", " 
            << variable.value_.matrix_float_4x4[2] << ", " 
            << variable.value_.matrix_float_4x4[3]
            << ")"
            << ", ("
            << variable.value_.matrix_float_4x4[4] << ", " 
            << variable.value_.matrix_float_4x4[5] << ", " 
            << variable.value_.matrix_float_4x4[6] << ", " 
            << variable.value_.matrix_float_4x4[7]
            << ")" 
            << ", ("
            << variable.value_.matrix_float_4x4[8] << ", " 
            << variable.value_.matrix_float_4x4[9] << ", " 
            << variable.value_.matrix_float_4x4[10] << ", " 
            << variable.value_.matrix_float_4x4[11]
            << ")" 
            << ", ("
            << variable.value_.matrix_float_4x4[12] << ", " 
            << variable.value_.matrix_float_4x4[13] << ", " 
            << variable.value_.matrix_float_4x4[14] << ", " 
            << variable.value_.matrix_float_4x4[15]
            << ")" << std::endl;
  variable.type_ = GL_FLOAT_MAT4;
}

typedef ShaderUniformVariable* _ShaderUniformVariablePtr;
template <>
void SetUniformValue(ShaderUniformVariable& variable, const _ShaderUniformVariablePtr& value) {
  variable.value_.value_self_ptr = value;
  std::cerr << "SetUniformValue - ShaderUniformVariable*: " << variable.value_.value_self_ptr << std::endl;
  variable.is_pointer_ = true;
}


} // cumbersome
