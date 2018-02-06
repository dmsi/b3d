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

#ifndef _MATERIAL_H_E93347A7_14DC_4024_A48C_D6B82C8123A6_
#define _MATERIAL_H_E93347A7_14DC_4024_A48C_D6B82C8123A6_ 

#include "texture.h"
#include "shader.h"
#include "pass.h"

#include <memory>
#include <string>
#include <vector>

class Pass;
class Material;

//////////////////////////////////////////////////////////////////////////////
// Material, Sir!
// Takes/shares ownership over shader (indirectly over passes) and textures.
//
//////////////////////////////////////////////////////////////////////////////
class Material {
 public:
  // TODO Hardcoded!
  enum { kMaxTextureSlots = 32 };
  
  Material() : textures_ (kMaxTextureSlots), name_("Unnamed") {}
  virtual ~Material() {}

  void Bind(); 
  void Unbind();
  void SetName(const std::string& name) { name_ = name; }
  void AddPass(std::shared_ptr<Pass> pass);
  void SetTexture(int slot, std::shared_ptr<Texture> texture);
  std::shared_ptr<Texture> GetTexture(int slot) {
    return textures_.at(slot);
  }

  template <typename T>
  void SetUniform(const std::string& name, const T& value) {
    for (auto& pass: pass_) {
      pass->SetUniform(name, value);
    }
  }

  template <typename T>
  void SetUniformArray(const std::string& name, const T values[], size_t n_values) {
    for (auto& pass: pass_) {
      pass->SetUniformArray(name, values, n_values);
    }
  }

  std::shared_ptr<Pass> GetPass(const std::string& name) const;
  std::shared_ptr<Pass> GetPass(int number) const;

  size_t GetPassNumber() const {
    return pass_.size();
  }

  const std::string& GetName() const {return name_;}

  // Iterations over passes
  auto begin() {
    return pass_.begin();
  }
  auto end() {
    return pass_.end();
  }

 private:
  using TexturePtr = std::shared_ptr<Texture>;
  std::vector<TexturePtr>               textures_;
  std::vector<std::shared_ptr<Pass>>    pass_;
  std::string                           name_;
};


#endif // _MATERIAL_H_E93347A7_14DC_4024_A48C_D6B82C8123A6_
