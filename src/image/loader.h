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

#ifndef _LOADER_H_U0_
#define _LOADER_H_U0_ 

#include "image/colormap.h"
#include <memory>
#include <string>

namespace Image {

std::shared_ptr<ColorMap> Load(const std::string& filename);
std::shared_ptr<ColorMap> Fill(int width, int height, const Color& color);

inline std::shared_ptr<ColorMap> Black(int width = 8, int height = 8) {
  return Fill(width, height, Color(0.0f, 0.0f, 0.0f, 1.0f));
}

inline std::shared_ptr<ColorMap> White(int width = 8, int height = 8) {
  return Fill(width, height, Color(1.0f, 1.0f, 1.0f, 1.0f));
}

} // namespace Image

#endif // _LOADER_H_U0_
