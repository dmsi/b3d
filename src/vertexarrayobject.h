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

#ifndef _VERTEXARRAYOBJECT_H_3757C313_50A4_4E57_A091_C276B99E84DB_
#define _VERTEXARRAYOBJECT_H_3757C313_50A4_4E57_A091_C276B99E84DB_ 

#include "gl_main.h"
#include "glm_main.h"
#include <cstdint>
#include <vector>
#include <algorithm>
#include <string>
#include <stdexcept>

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
class VertexArrayObject {
 public:
  enum { kMaxAttributeSlots = 16 };

  ////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////
  struct PackedData {
    const void*  data;
    std::size_t  data_size;
    GLint        num_components;
    GLenum       type;

    PackedData(const void* _data, std::size_t _data_size, GLint _num_components, GLenum _type) 
      : data(_data), data_size(_data_size), num_components(_num_components), type(_type) {}

    // glm::vecx is c++ standard type layout class (xyz order ?)
    static PackedData Pack(const std::vector<uint16_t>& data) {
      // 1 and GL_SHORT are just ignored now, 16 bit indices only 
      return PackedData(&data.at(0), sizeof(uint16_t)*data.size(), 1, GL_SHORT);
    }
    static PackedData Pack(const std::vector<glm::vec2>& data) {
      return PackedData(&data.at(0), sizeof(glm::vec2)*data.size(), 2, GL_FLOAT);
    }
    static PackedData Pack(const std::vector<glm::vec3>& data) {
      return PackedData(&data.at(0), sizeof(glm::vec3)*data.size(), 3, GL_FLOAT);
    }
    static PackedData Pack(const std::vector<glm::vec4>& data) {
      return PackedData(&data.at(0), sizeof(glm::vec4)*data.size(), 4, GL_FLOAT);
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  // New VAO needs to be created in order to switch from static to dynamic and 
  // vice-versa.
  //////////////////////////////////////////////////////////////////////////////
  VertexArrayObject(bool static_usage = true) 
    : vao_          (0), 
      vbo_          (kMaxAttributeSlots), 
      indices_vbo_  (0), 
      static_       (static_usage) {
    std::fill(vbo_.begin(), vbo_.end(), VboData());
    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);
  }

  virtual ~VertexArrayObject() {
    for (auto vbo: vbo_) {
      if (vbo.vbo_id) {
        glDeleteBuffers(1, &vbo.vbo_id);
      }
    }
    if (indices_vbo_) {
      glDeleteBuffers(1, &indices_vbo_);
    }
    if (vao_ != 0) {
      glDeleteVertexArrays(1, &vao_);
    }
  }
  
  //////////////////////////////////////////////////////////////////////////////
  // Upload data to video memory, to attribute slot
  //////////////////////////////////////////////////////////////////////////////
  void Upload(int attrib_slot, PackedData data) {
    if (attrib_slot < 0 || attrib_slot >= (int)vbo_.size()) {
      throw std::logic_error("VertexArrayObject::FillBuffer() bad attrib_slot=" + std::to_string(attrib_slot));
    }

    if (vbo_[attrib_slot].vbo_id == 0) {
      GLuint vbo_id;
      glGenBuffers(1, &vbo_id);
      vbo_[attrib_slot] = VboData(vbo_id, data.num_components, data.type);
    }
    GLuint vbo_id = vbo_[attrib_slot].vbo_id;
    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
    glBufferData(GL_ARRAY_BUFFER, data.data_size, data.data, static_ ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW); 
    glVertexAttribPointer(attrib_slot, data.num_components, data.type, GL_FALSE, 0, nullptr);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

  //////////////////////////////////////////////////////////////////////////////
  // Upload indices to video memory
  //////////////////////////////////////////////////////////////////////////////
  //void UploadIndices(const uint16_t* indices, std::size_t num_indices) {
  void UploadIndices(PackedData indices) {
    if (indices_vbo_ == 0) {
      glGenBuffers(1, &indices_vbo_);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_vbo_);
    }

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.data_size, 
                 indices.data, static_ ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  }

  //////////////////////////////////////////////////////////////////////////////
  // Bind for the draw call 
  //////////////////////////////////////////////////////////////////////////////
  void Bind() {
    glBindVertexArray(vao_);
    for (int i = 0; i < (int)vbo_.size(); ++i) {
      VboData& the_vbo = vbo_[i];
      if (the_vbo.vbo_id) {
        glEnableVertexAttribArray(i);
      }
    }
    if (indices_vbo_) {
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_vbo_);
    }
  }

  //////////////////////////////////////////////////////////////////////////////
  // Unbinds after the draw call 
  //////////////////////////////////////////////////////////////////////////////
  void Unbind() {
    for (int i = 0; i < (int)vbo_.size(); ++i) {
      VboData& the_vbo = vbo_[i];
      if (the_vbo.vbo_id) {
        glDisableVertexAttribArray(i);
      }
    }
    if (indices_vbo_) {
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
    glBindVertexArray(vao_);
  }

 private:
  struct VboData {
    GLuint vbo_id;
    GLint  num_components;
    GLenum type;

    VboData(GLuint _vbo_id = 0, GLint _num_components = 0, GLenum _type = GL_FLOAT) 
      : vbo_id(_vbo_id), num_components(_num_components), type(_type) {}
  };


  GLuint                    vao_; 
  std::vector<VboData>      vbo_;
  GLuint                    indices_vbo_;
  bool                      static_;
};

#endif // _VERTEXARRAYOBJECT_H_3757C313_50A4_4E57_A091_C276B99E84DB_
