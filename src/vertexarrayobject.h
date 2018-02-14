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
#include "attributelayout.h"
#include "common/logging.h"
#include <bitset>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <string>
#include <stdexcept>

#include <iostream>

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
class VertexArrayObject {
 public:
  // Modern OpenGL requirement for vendors is minimun 16 attribute slots
  enum { 
    kVboSlots1a = 8,               // 1 VBO => single attribute
    kVboSlotsNa = kVboSlots1a + 8, // 1 VBO => multi attributes
    kMaxAttributeSlots = kVboSlotsNa 
  };

  enum Usage {
    kUsageStatic = GL_STATIC_DRAW,   // upload once, use many times
    kUsageStream = GL_STREAM_DRAW,   // upload once, use few times
    kUsageDynamic = GL_DYNAMIC_DRAW, // upload once, use once
  };

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
      return PackedData(&data.at(0), sizeof(uint16_t)*data.size(), 1, GL_UNSIGNED_SHORT);
    }
    static PackedData Pack(const std::vector<uint32_t>& data) {
      // 1 and GL_SHORT are just ignored now, 16 bit indices only 
      return PackedData(&data.at(0), sizeof(uint32_t)*data.size(), 1, GL_UNSIGNED_INT);
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
    std::fill(vbo_.begin(), vbo_.end(), VboInfo{});
    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);
  }

  virtual ~VertexArrayObject() {
    for (auto& vbo: vbo_) {
      if (vbo.id) {
        glDeleteBuffers(1, &vbo.id);
      }
      vbo = VboInfo{};
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
    if (attrib_slot < 0 || attrib_slot >= kVboSlots1a) { 
      ABORT_F("Bad attribute slot %d", attrib_slot);
    }

    auto& vbo = vbo_[attrib_slot];
    if (vbo.IsEmpty()) {
      GLuint id;
      glGenBuffers(1, &id);
      vbo = VboInfo{id, data.num_components, data.type, data.data_size, attrib_slot, 1};
    }

    glBindBuffer(GL_ARRAY_BUFFER, vbo.id);
    glBufferData(GL_ARRAY_BUFFER, data.data_size, data.data, static_ ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW); 
    glVertexAttribPointer(attrib_slot, data.num_components, data.type, GL_FALSE, 0, nullptr);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

  // Per instance attributes allocation 
  template <typename TLayout>
  void Allocate(int start, size_t n_elements, void* data, Usage usage) {
    assert(start >= kVboSlots1a && start < kVboSlotsNa);

    constexpr auto total = TLayout::Attributes();

    assert(VboHasRoom(start, total));
    auto& vbo = NewVbo(start, total); 
    assert(vbo.IsEmpty());

    size_t buff_sz = TLayout::Stride() * n_elements;

    glBindVertexArray(vao_);

    GLuint id;
    glGenBuffers(1, &id);
    vbo = VboInfo{id, 0, 0, buff_sz, start, total};
    glBindBuffer(GL_ARRAY_BUFFER, vbo.id);
    glBufferData(GL_ARRAY_BUFFER, buff_sz, data, usage); 

    never_easy::for_<total>([&] (auto i) {
        using TAttr = typename TLayout::template TAttribute<i.value>; 
        auto offset = TLayout::template Offset<i.value>();

        glVertexAttribPointer(
            start + i.value, 
            AttributeTraits<TAttr>::n_components,
            AttributeTraits<TAttr>::type, 
            GL_FALSE, 
            TLayout::Stride(), 
            (void*)offset);

        glVertexAttribDivisor(start + i.value, 1);
    }); // for_<N>
  
    glBindVertexArray(0);
  }

  // Per instance upload
  template <typename TLayout>
  void Upload(int start, size_t n_elements, void* data, Usage usage) {
    assert(start >= kVboSlots1a && start < kVboSlotsNa);
    
    constexpr auto total = TLayout::Attributes();

    auto& vbo = GetVbo(start, total); 

    if (vbo.IsEmpty()) {
      Allocate<TLayout>(start, n_elements, data, usage);
    } else {
      size_t buff_sz = TLayout::Stride() * n_elements;
      glBindBuffer(GL_ARRAY_BUFFER, vbo.id);
      glBufferData(GL_ARRAY_BUFFER, buff_sz, data, usage); 
    }
  }
  
  // Per instance attribute memory map
  template <typename TLayout>
  BufferView Map(int start, size_t n_elements) {
    assert(start >= kVboSlots1a && start < kVboSlotsNa);
    
    constexpr auto total = TLayout::Attributes();

    auto& vbo = GetVbo(start, total); 

    if (vbo.IsEmpty()) {
      Allocate<TLayout>(start, n_elements, nullptr, kUsageStream);
    } else {
      glBindBuffer(GL_ARRAY_BUFFER, vbo.id);
    }
    
    void* ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    return BufferView(ptr, TLayout::Stride() * n_elements);
  }

  // Per instance attribute memory unmap 
  void Unmap(int attrib_slot) {
    glUnmapBuffer(GL_ARRAY_BUFFER);
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
  
    for (auto& v : vbo_) {
      v.Enable(); 
    }

    if (indices_vbo_) {
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_vbo_);
    }
  }

  //////////////////////////////////////////////////////////////////////////////
  // Unbinds after the draw call 
  //////////////////////////////////////////////////////////////////////////////
  void Unbind() {
    for (auto& v: vbo_) {
      v.Disable();
    }

    if (indices_vbo_) {
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    glBindVertexArray(vao_);
  }

  // checks for attribute slots intersections
  bool IsValid() const {
    assert(false && "not implemented");
    return true;
  }

 private:
  struct VboInfo {
    GLuint id           = 0;
    GLint  n_components = 0;  // when total == 1
    GLenum type         = GL_FLOAT;
    size_t size         = 0;
    int    start        = -1; // start VAO slot
    int    total        = -1; // total VAO slots

    VboInfo() = default;

    void Enable() const {
      for (int i = 0; i < total; ++i) {
        glEnableVertexAttribArray(start + i);
      }
    }

    void Disable() const {
      for (int i = 0; i < total; ++i) {
        glDisableVertexAttribArray(start + i);
      }
    }

    bool IsValid() const {
      if (id > 0) {
        return (start >= 0 && total > 0) && n_components > 0 && size > 0;
      } else {
        return true;
      }
    }

    bool IsEmpty() const {
      return id == 0;
    }
  };
  
  VboInfo& GetVbo(int start, int total) {
    //assert(vbo_bitset_[start] == 1);
    return vbo_[start];
  }

  VboInfo& NewVbo(int start, int total) {
    assert(start >= 0 && start < kMaxAttributeSlots);
    assert(total >= 1);
    assert(start + total < kMaxAttributeSlots);
    // Check if start is free
    assert(vbo_bitset_[start] == 0);
    for (int i = 0; i < total; ++i) {
      assert(vbo_bitset_[start + i] == 0);
      vbo_bitset_[start + i] = 1;
    }
    return vbo_[start];
  }

  bool VboHasRoom(int start, int total) const {
    if (start >= 0 && start <kMaxAttributeSlots &&
        total >= 1 && start + total < kMaxAttributeSlots) {

      if (start < kVboSlots1a && total > 1)
        return false;

      for (int i = 0; i < total; i++)
        if (vbo_bitset_[start+i])
          return false;
    }

    return true;
  }
  
  GLuint                          vao_; 
  std::vector<VboInfo>            vbo_;
  std::bitset<kMaxAttributeSlots> vbo_bitset_;
  GLuint                          indices_vbo_;
  bool                            static_;
};

#endif // _VERTEXARRAYOBJECT_H_3757C313_50A4_4E57_A091_C276B99E84DB_
