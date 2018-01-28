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

#ifndef _MATERIAL_LOADER_H_1F3034F9_92C2_44C5_9A84_73CF8F2D9552_
#define _MATERIAL_LOADER_H_1F3034F9_92C2_44C5_9A84_73CF8F2D9552_ 

#include "material/material.h"
#include "material/pass.h"
#include <memory>
#include <string>

//////////////////////////////////////////////////////////////////////////////
// Load material stored in YAML file
//////////////////////////////////////////////////////////////////////////////
class MaterialLoader {
 public:
  static std::shared_ptr<Material> Load(const std::string& filename);
};

#endif // _MATERIAL_LOADER_H_1F3034F9_92C2_44C5_9A84_73CF8F2D9552_
