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

#ifndef _MESHLOADER_H_8ADA9A55_E5DE_44B4_8D4A_156B7A4808CA_
#define _MESHLOADER_H_8ADA9A55_E5DE_44B4_8D4A_156B7A4808CA_ 

#include "meshfilter.h"
#include "glm_main.h"
#include <memory>
#include <string>

class MeshLoader {
 public:
  static
  std::shared_ptr<Mesh> Load(const std::string& filename) {
    std::string ext = GetFileExt(filename);
    if (ext == "dsm") {
      return LoadDsm(filename);
    }

    return nullptr;
  }

  static 
  void ExportDsm(std::shared_ptr<Mesh> mesh, const std::string& filename);

 private:
  static
  std::string GetFileExt(const std::string& filename) {
    auto pos = filename.rfind('.');
    if (pos != std::string::npos) {
      return filename.substr(pos + 1);
    } else {
      return std::string();
    }
  }

  static std::shared_ptr<Mesh> LoadDsm(const std::string& filename);
};

#endif // _MESHLOADER_H_8ADA9A55_E5DE_44B4_8D4A_156B7A4808CA_
