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

#include "image/loader.h"
#include "image/portablepixmap.h"
#include "common/logging.h"
#include <stdexcept>

namespace Image {

std::shared_ptr<ColorMap> Load(const std::string& filename) { 
  std::string ext = filename.substr(filename.find_last_of(".") + 1);
  if (ext == "ppm" || ext == "pgm" || ext == "pam") {
    return PortablePixMap::Read(filename);
  } else {
    ABORT_F("Cant load image %s. Format not supported", filename.c_str());
  }
}

std::shared_ptr<ColorMap> Fill(int width, int height, const Color& color) {
  std::shared_ptr<ColorMap> cmap(new ColorMap(width, height));
  for (size_t i = 0; i < cmap->GetLength(); ++i) {
    cmap->At(i) = color;
  }
  return cmap;
}

} // namespace Image
