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

#include "material/material.h"
#include "common/logging.h"
#include <algorithm>
  
//////////////////////////////////////////////////////////////////////////////
// Material
//////////////////////////////////////////////////////////////////////////////

void Material::Bind() {
  for (int i = 0; i < (int)textures_.size(); ++i) {
    if (textures_[i]) {
      textures_[i]->Bind(i);
    }
  }
}

void Material::Unbind() {
  for (int i = 0; i < (int)textures_.size(); ++i) {
    if (textures_[i]) {
      textures_[i]->Unbind(i);
    }
  }
}

void Material::AddPass(std::shared_ptr<Pass> pass) {
  pass_.push_back(pass);
}

void Material::SetTexture(int slot, std::shared_ptr<Texture> texture) {
  if (slot < 0 || slot > (int)textures_.size()) {
    ABORT_F("Invalid texture slot %d", slot);
  }
  textures_[slot] = texture;
}

std::shared_ptr<Pass> Material::GetPass(const std::string& name) const {
  auto it = std::find_if(
      pass_.begin(),
      pass_.end(),
      [&name](const std::shared_ptr<Pass> p) {
        return p->GetName() == name;
      });

  if (it == pass_.end()) {
    ABORT_F("Pass %s not found", name.c_str());
  }

  return *it;
}

std::shared_ptr<Pass> Material::GetPass(int number) const {
  return pass_.at(number);
}


