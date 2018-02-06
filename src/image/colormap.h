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

#ifndef _COLORMAP_H_58E91C6F_11CB_46DE_9BB8_646EFC33A1B9_
#define _COLORMAP_H_58E91C6F_11CB_46DE_9BB8_646EFC33A1B9_ 

#include "common/array2d.h"
#include "glm_main.h"
#include "math_main.h"
#include <memory>

typedef glm::vec4           Color;
typedef glm::tvec4<uint8_t> Color32; 

// -> #include "color.h"
inline Color Rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
  return Color(r/255., g/255., b/255., a/255.);
}

inline Color Rgb(uint8_t r, uint8_t g, uint8_t b) {
  return Rgba(r, g, b, 1); 
}

namespace Image {

typedef Array2d<Color> ColorMap;
typedef Array2d<Color32> ColorMap32;


inline std::shared_ptr<ColorMap32> Convert(const ColorMap& cmap) {
  std::shared_ptr<ColorMap32> cmap32(new ColorMap32(cmap.GetWidth(), cmap.GetHeight()));
  for(size_t i = 0; i < cmap.GetWidth() * cmap.GetHeight(); ++i) {
    const Color& c = cmap.At(i);
    cmap32->At(i) = {(uint8_t)Math::Clamp(0.0f, 255.0f, c.r*255),
                     (uint8_t)Math::Clamp(0.0f, 255.0f, c.g*255),
                     (uint8_t)Math::Clamp(0.0f, 255.0f, c.b*255),
                     (uint8_t)Math::Clamp(0.0f, 255.0f, c.a*255)};
  }
  return cmap32;
}

inline std::shared_ptr<ColorMap> Convert(const ColorMap32& cmap32) {
  std::shared_ptr<ColorMap> cmap(new ColorMap(cmap32.GetWidth(), cmap32.GetHeight()));
  for(size_t i = 0; i < cmap32.GetWidth() * cmap32.GetHeight(); ++i) {
    const Color32& c = cmap32.At(i);
    cmap->At(i) = {(uint8_t)Math::Clamp(0.0f, 1.0f, c.r/255.0f),
                   (uint8_t)Math::Clamp(0.0f, 1.0f, c.g/255.0f),
                   (uint8_t)Math::Clamp(0.0f, 1.0f, c.b/255.0f),
                   (uint8_t)Math::Clamp(0.0f, 1.0f, c.a/255.0f)};
  }
  return cmap;
}

void Compress(const ColorMap& cmap, std::vector<uint8_t>& compressed);
void Compress(const ColorMap32& cmap, std::vector<uint8_t>& compressed);

} // Image

#endif // _COLORMAP_H_58E91C6F_11CB_46DE_9BB8_646EFC33A1B9_
