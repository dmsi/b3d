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

#ifndef _PORTABLEPIXMAP_H_CE0D2532_D3EF_467B_915A_5F0D8C40CC88_
#define _PORTABLEPIXMAP_H_CE0D2532_D3EF_467B_915A_5F0D8C40CC88_ 

#include "colormap.h"
#include <vector>
#include <string>
#include <memory>

namespace Image {

//////////////////////////////////////////////////////////////////////////////
// http://netpbm.sourceforge.net/doc/ppm.html
//
// Limitations:
//  - P6, P5 formats (PPM and PGM)
//    - Reads the first image from file (if more than one)
//    - Maximum color value is 255 per channel
//  - P7 format (PAM)
//    - TUPLTYPE requried
//    - TUPLTYPE GRAYSCALE and DEPTH 1
//    - TUPLTYPE RGB and DEPTH 3 
//    - TUPLTYPE RGB_ALPHA and DEPTH 4
//    - Maximum color value is 255 per channel
//////////////////////////////////////////////////////////////////////////////
class PortablePixMap {
 public:
  static std::shared_ptr<ColorMap> Read(const std::string& filename);
  static void Write(const std::string& filename, const ColorMap& img);
};

} // namespace Image

#endif // _PORTABLEPIXMAP_H_CE0D2532_D3EF_467B_915A_5F0D8C40CC88_
