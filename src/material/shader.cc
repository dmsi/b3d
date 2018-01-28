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

#include "shader.h"

#include <typeinfo>


inline
std::string GetProgramInfoLog(GLuint id, bool compilation=false) {
  char buffer[256] = {0};
  GLsizei log_length;
  if (compilation) {
    glGetProgramInfoLog(id, sizeof(buffer), &log_length, buffer);
  } else {
    glGetShaderInfoLog(id, sizeof(buffer), &log_length, buffer);
  }
  return std::string(buffer);
}

//////////////////////////////////////////////////////////////////////////////
// ShaderCompiler. Keep max str length under control!
//////////////////////////////////////////////////////////////////////////////

ShaderCompiler::ShaderCompiler(ShaderCompiler::ShaderType type, 
                               const std::string& source_code) 
  : type_(type), shader_id_(0) {
  // Beauty separator
  shader_id_ = glCreateShader(type);

  const char* code_ptr = source_code.c_str();
  glShaderSource(shader_id_, 1, &code_ptr, nullptr);
  glCompileShader(shader_id_);

  GLint result = GL_FALSE;
  glGetShaderiv(shader_id_, GL_COMPILE_STATUS, &result);
  if (result == GL_FALSE) {
    std::string error = GetProgramInfoLog(shader_id_);
    glDeleteShader(shader_id_);
    shader_id_ = 0;
    throw std::runtime_error("ShaderCompiler() cant compile shader!\n" + error);
  } 
}

ShaderCompiler::~ShaderCompiler() {
  if (shader_id_) {
    glDeleteShader(shader_id_);
    shader_id_ = 0;
  }
}
  

//////////////////////////////////////////////////////////////////////////////
// Shader. 
//////////////////////////////////////////////////////////////////////////////

Shader::Shader() : program_id_(0) {}

Shader::~Shader() {
  if (program_id_) {
    glDeleteProgram(program_id_);
  }
}

void Shader::Compile(ShaderCompiler::ShaderType type, const std::string& code) {
  ShaderCompilerPtr compiler(new ShaderCompiler(type, code));
  shader_compilers_[compiler->GetShaderId()] = compiler;
}

void Shader::Link() {
  if (shader_compilers_.empty()) {
    throw std::logic_error("Shader::Link() - shader must be compiled first!");
  }

  if (program_id_) {
    glDeleteProgram(program_id_);
  }
  program_id_ = glCreateProgram();

  // Attach shader pieces (way clearer than for_each + lambda!)
  for (auto& kv : shader_compilers_) {
    glAttachShader(program_id_, kv.second->GetShaderId());
  }


  // Linking happens here
  GLint result;
  glLinkProgram(program_id_);
  glGetProgramiv(program_id_, GL_LINK_STATUS, &result);
  if (result == GL_FALSE) {

    std::string error = GetProgramInfoLog(program_id_, true);
    //std::cerr << "**FUCK FUCK FUCK**" << std::endl;
    //std::cerr << "**WTF: " << GetProgramInfoLog(program_id_) << std::endl;

    for (auto& kv : shader_compilers_) {
      glDetachShader(program_id_, kv.second->GetShaderId());
    }
    glDeleteProgram(program_id_);
    program_id_ = 0;
    throw std::runtime_error("Shader::Link() cant link shader!\n" + error);
  } 

  // Detach shader pieces.
  for (auto& kv : shader_compilers_) {
    glDetachShader(program_id_, kv.second->GetShaderId());
  }

  RebuildUniforms();

  // Dont need the compiled parts anymore
  shader_compilers_.clear();
}

void Shader::PrintInfo() const {
  std::cerr << "Shader program | " << program_id_ << std::endl;
  for (auto& kv : uniforms_) {
    const UniformInfo& u = kv.second;
    std::cerr << "             " << u.location << " | " << u.name << " " 
              << VariableTypeToString(u.type) << std::endl;
  }
}
  
void Shader::RebuildUniforms() {
  if (program_id_ == 0) {
    throw std::logic_error("Shader::RebuildUniforms() - shader must be compiled first!");
  }

  uniforms_.clear();

  GLint total_uniforms;
  const GLsizei max_uniform_name_lenth = 128;
  GLsizei actual_uniform_name_length;
  char uniform_name[max_uniform_name_lenth] = {0};
  GLint uniform_array_length;
  GLenum uniform_type;
        
  glGetProgramiv(program_id_, GL_ACTIVE_UNIFORMS, &total_uniforms);
  for (GLint i = 0; i < total_uniforms; ++i) {
    glGetActiveUniform(
        program_id_,                   // program id (vertex, geom, frag, etc.. together)
        i,                             // this is not location, just an index, use glGetUniformLocation
        max_uniform_name_lenth,        // uniform name cap
        &actual_uniform_name_length,   // actual uniform name (up to cap)
        &uniform_array_length,         // 1 for single, more for arrays
        &uniform_type,                 // uniform type
        uniform_name                   // uniform name
        );
    if (uniform_array_length != 1) {
      uniforms_.clear();
      throw std::logic_error("Shader::RebuildUniforms() array uniforms are not supported!");
    }
    GLint location = glGetUniformLocation(program_id_, uniform_name);
    // what if uniform_name is the same in VS and FS?
    uniforms_[uniform_name] = UniformInfo(location, uniform_name, uniform_type);
  }
}

std::string Shader::VariableTypeToString(uint32_t type) {
  switch(type) {
    case GL_FLOAT: return "float";
    case GL_FLOAT_VEC2: return "vec2";
    case GL_FLOAT_VEC3: return "vec3";
    case GL_FLOAT_VEC4: return "vec4";
    case GL_DOUBLE: return "double";
    case GL_DOUBLE_VEC2: return "dvec2";
    case GL_DOUBLE_VEC3: return "dvec3";
    case GL_DOUBLE_VEC4: return "dvec4";
    case GL_INT: return "int";
    case GL_INT_VEC2: return "ivec2";
    case GL_INT_VEC3: return "ivec3";
    case GL_INT_VEC4: return "ivec4";
    case GL_UNSIGNED_INT: return "unsigned int";
    case GL_UNSIGNED_INT_VEC2: return "uvec2";
    case GL_UNSIGNED_INT_VEC3: return "uvec3";
    case GL_UNSIGNED_INT_VEC4: return "uvec4";
    case GL_BOOL: return "bool";
    case GL_BOOL_VEC2: return "bvec2";
    case GL_BOOL_VEC3: return "bvec3";
    case GL_BOOL_VEC4: return "bvec4";
    case GL_FLOAT_MAT2: return "mat2";
    case GL_FLOAT_MAT3: return "mat3";
    case GL_FLOAT_MAT4: return "mat4";
    case GL_FLOAT_MAT2x3: return "mat2x3";
    case GL_FLOAT_MAT2x4: return "mat2x4";
    case GL_FLOAT_MAT3x2: return "mat3x2";
    case GL_FLOAT_MAT3x4: return "mat3x4";
    case GL_FLOAT_MAT4x2: return "mat4x2";
    case GL_FLOAT_MAT4x3: return "mat4x3";
    case GL_DOUBLE_MAT2: return "dmat2";
    case GL_DOUBLE_MAT3: return "dmat3";
    case GL_DOUBLE_MAT4: return "dmat4";
    case GL_DOUBLE_MAT2x3: return "dmat2x3";
    case GL_DOUBLE_MAT2x4: return "dmat2x4";
    case GL_DOUBLE_MAT3x2: return "dmat3x2";
    case GL_DOUBLE_MAT3x4: return "dmat3x4";
    case GL_DOUBLE_MAT4x2: return "dmat4x2";
    case GL_DOUBLE_MAT4x3: return "dmat4x3";
    case GL_SAMPLER_1D: return "sampler1D";
    case GL_SAMPLER_2D: return "sampler2D";
    case GL_SAMPLER_3D: return "sampler3D";
    case GL_SAMPLER_CUBE: return "samplerCube";
    case GL_SAMPLER_1D_SHADOW: return "sampler1DShadow";
    case GL_SAMPLER_2D_SHADOW: return "sampler2DShadow";
    case GL_SAMPLER_1D_ARRAY: return "sampler1DArray";
    case GL_SAMPLER_2D_ARRAY: return "sampler2DArray";
    case GL_SAMPLER_1D_ARRAY_SHADOW: return "sampler1DArrayShadow";
    case GL_SAMPLER_2D_ARRAY_SHADOW: return "sampler2DArrayShadow";
    case GL_SAMPLER_2D_MULTISAMPLE: return "sampler2DMS";
    case GL_SAMPLER_2D_MULTISAMPLE_ARRAY: return "sampler2DMSArray";
    case GL_SAMPLER_CUBE_SHADOW: return "samplerCubeShadow";
    case GL_SAMPLER_BUFFER: return "samplerBuffer";
    case GL_SAMPLER_2D_RECT: return "sampler2DRect";
    case GL_SAMPLER_2D_RECT_SHADOW: return "sampler2DRectShadow";
    case GL_INT_SAMPLER_1D: return "isampler1D";
    case GL_INT_SAMPLER_2D: return "isampler2D";
    case GL_INT_SAMPLER_3D: return "isampler3D";
    case GL_INT_SAMPLER_CUBE: return "isamplerCube";
    case GL_INT_SAMPLER_1D_ARRAY: return "isampler1DArray";
    case GL_INT_SAMPLER_2D_ARRAY: return "isampler2DArray";
    case GL_INT_SAMPLER_2D_MULTISAMPLE: return "isampler2DMS";
    case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY: return "isampler2DMSArray";
    case GL_INT_SAMPLER_BUFFER: return "isamplerBuffer";
    case GL_INT_SAMPLER_2D_RECT: return "isampler2DRect";
    case GL_UNSIGNED_INT_SAMPLER_1D: return "usampler1D";
    case GL_UNSIGNED_INT_SAMPLER_2D: return "usampler2D";
    case GL_UNSIGNED_INT_SAMPLER_3D: return "usampler3D";
    case GL_UNSIGNED_INT_SAMPLER_CUBE: return "usamplerCube";
    case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY: return "usampler2DArray";
    case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY: return "usampler2DArray";
    case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE: return "usampler2DMS";
    case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY: return "usampler2DMSArray";
    case GL_UNSIGNED_INT_SAMPLER_BUFFER: return "usamplerBuffer";
    case GL_UNSIGNED_INT_SAMPLER_2D_RECT: return "usampler2DRect";
    default: return std::to_string(type);
  }
}
