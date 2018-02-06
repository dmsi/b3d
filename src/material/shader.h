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

#ifndef _SHADER_H_13CCC4E1_0635_4DB7_8334_A8318AFB89F8_
#define _SHADER_H_13CCC4E1_0635_4DB7_8334_A8318AFB89F8_ 

#include "gl_main.h"
#include "glm_main.h"
#include "common/logging.h"

#include <exception>
#include <memory>
#include <string>
#include <map>
#include <iostream>
#include <string.h> // memcpy

//////////////////////////////////////////////////////////////////////////////
// C++-ness!
//////////////////////////////////////////////////////////////////////////////
namespace Shader_cppness {
  template <typename T>
  inline
  void SetUniform(GLint location, const T& value, GLenum expected_type); // compile time error

  template <typename T>
  inline
  void SetUniformArray(GLint location, const T values[], size_t n_values, GLenum expected_type);

  template <>
  inline
  void SetUniform<float>(GLint location, const float& value, GLenum expected_type) {
    // This is how the validation can be added
    if (expected_type != GL_FLOAT) {
      ABORT_F("Bad type");
    }
    glUniform1f(location, value);
  }
  
  template <>
  inline
  void SetUniform<int>(GLint location, const int& value, GLenum expected_type) {
    glUniform1i(location, value);
  }
  

  template <>
  inline
  void SetUniform<glm::mat4>(GLint location, const glm::mat4& mat, GLenum expected_type) {
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat));
  }

  template <>
  inline 
  void SetUniform<glm::vec4>(GLint location, const glm::vec4& vec, GLenum expected_type) {
    glUniform4fv(location, 1, &vec[0]);
  }

  template <>
  inline 
  void SetUniform<glm::vec3>(GLint location, const glm::vec3& vec, GLenum expected_type) {
    glUniform3fv(location, 1, &vec[0]);
  }
  
  template <>
  inline 
  void SetUniform<glm::vec2>(GLint location, const glm::vec2& vec, GLenum expected_type) {
    glUniform2fv(location, 1, &vec[0]);
  }
  
  template <>
  inline
  void SetUniformArray<int>(GLint location, const int values[], size_t n_values, GLenum expected_type) {
    glUniform1iv(location, n_values, values);
  }
}

//////////////////////////////////////////////////////////////////////////////
// Almost as cool as gcc, but scc... 
// VS, GS and FS are supported for now
//////////////////////////////////////////////////////////////////////////////
class ShaderCompiler {
 public:
  enum ShaderType {
    kVertexShader   = GL_VERTEX_SHADER, 
    kGeometryShader = GL_GEOMETRY_SHADER, 
    kFragmentShader = GL_FRAGMENT_SHADER,
    kEmpty
  };
 
  ShaderCompiler(ShaderType type, const std::string& source_code);
  virtual ~ShaderCompiler();

  ShaderType GetShaderType() const { return type_; }
  uint32_t   GetShaderId()   const { return shader_id_; }

 private:
  ShaderType     type_;
  GLuint         shader_id_;
};

//////////////////////////////////////////////////////////////////////////////
// Shader with blackjack and hookers.
//
// Shader sox;
// sox.Compile(ReadFile("sox_vs.glsl"));
// sox.Compile(ReadFile("sox_fs.glsl"));
// sox.Link();
// ...
// sox.Bind();
// ...
// DrawCall();
// sox.Unbind();
//////////////////////////////////////////////////////////////////////////////
class Shader {
 public:
  Shader();
  virtual ~Shader();

  /////////////////////////////////////////////////////////////////////////////
  // Compiles part of shader (VS, GS, FS). Replaces if exist. 
  /////////////////////////////////////////////////////////////////////////////
  void Compile(ShaderCompiler::ShaderType type, const std::string& code);

  /////////////////////////////////////////////////////////////////////////////
  // Links all parts into a single program! Pieces must be added by calling
  // Shader::Compile() before this call. All shader pieces removed in case of 
  // successful linkage. 
  // Calling Link() multiple times will cause exception! 
  /////////////////////////////////////////////////////////////////////////////
  void Link();

  ////////////////////////////////////////////////////////////////////////////
  // Make it active / deactive for draw call
  ////////////////////////////////////////////////////////////////////////////
  void Bind() const {
    if (program_id_ == 0) {
      ABORT_F("Shader is not compiled and linked");
    }
    glUseProgram(program_id_);
  }

  void Unbind() const {
    glUseProgram(0);
  }

  ////////////////////////////////////////////////////////////////////////////
  // Sets uniform variable to the shader. Does nothing if no such variable. 
  ////////////////////////////////////////////////////////////////////////////
  template <typename T>
  void SetUniform(const std::string& name, const T& value) {
    // TODO this lookup for every single uniform is expensive need a faster
    // way of doing this. Direct int locations or uniform buffer instead...
    auto uniform_iter = uniforms_.find(name);
    if (uniform_iter != uniforms_.end()) {
      const UniformInfo& u = uniform_iter->second;
      Shader_cppness::SetUniform(u.location, value, u.type);
    }
  }

  template <typename T>
  void SetUniformArray(const std::string& name, const T values[], size_t n_values) {
    auto uniform_iter = uniforms_.find(name);
    if (uniform_iter != uniforms_.end()) {
      const UniformInfo& u = uniform_iter->second;
      Shader_cppness::SetUniformArray(u.location, values, n_values, u.type);
    }
  }

  uint32_t GetProgramId() const { return program_id_; }
  void PrintInfo() const;

  static std::string VariableTypeToString(uint32_t type);


 private:
  void RebuildUniforms();

  ////////////////////////////////////////////////////////////////////////////
  // Uniform variable info
  // rename to UniformInfo
  ////////////////////////////////////////////////////////////////////////////
  struct UniformInfo {
    int           location;
    std::string   name;
    GLenum        type;

    UniformInfo(int l=0, const std::string& n="", GLenum t=GL_FLOAT) 
      : location(l), name(n), type(t) { }
  };

  typedef std::shared_ptr<ShaderCompiler> ShaderCompilerPtr;

  std::map<GLuint, ShaderCompilerPtr>    shader_compilers_;
  std::map<std::string, UniformInfo>     uniforms_;
  GLuint                                 program_id_; 
};

#endif // _SHADER_H_13CCC4E1_0635_4DB7_8334_A8318AFB89F8_
