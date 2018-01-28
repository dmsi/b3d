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

#include "portablepixmap.h"
#include "math_main.h"
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iterator>

namespace Image {
  
struct PpmHeader {
  std::string version;
  size_t      width;
  size_t      height;
  uint16_t    maxval;
};

struct PamHeader {
  std::string version;
  size_t      width;
  size_t      height;
  size_t      depth;
  uint16_t    maxval;
  std::string tupltype;
};
  
static void SkipCommentLine(std::ifstream& in, std::string& line) {
  std::getline(in, line);
  while (line[0] == '#') {
    std::getline(in, line);
  }
}

static std::shared_ptr<ColorMap> ReadP6(std::ifstream& in, const PpmHeader& header) {
  std::shared_ptr<ColorMap> img(new ColorMap(header.width, header.height));
  size_t sz = header.width * header.height;
  float maxval = (float)header.maxval;
  for (size_t i = 0; i < sz; ++i) {
    uint8_t rgb[3]; 
    in.read((char*)rgb, 3);
    img->At(i) = Color(rgb[0]/maxval, rgb[1]/maxval, rgb[2]/maxval, 1);
  }
  return img;
}

static std::shared_ptr<ColorMap> ReadP3(std::ifstream& in, const PpmHeader& header) {
  throw std::logic_error("PortablePixMap::Read() - P3 format is not implemented!");
  return std::shared_ptr<ColorMap>();
}

static std::shared_ptr<ColorMap> ReadP5(std::ifstream& in, const PpmHeader& header) {
  std::shared_ptr<ColorMap> img(new ColorMap(header.width, header.height));
  size_t sz = header.width * header.height;
  float maxval = (float)header.maxval;
  for (size_t i = 0; i < sz; ++i) {
    uint8_t c;
    in.read((char*)&c, 1);
    img->At(i) = Color(c/maxval, c/maxval, c/maxval, 1);
  }
  return img;
}

static std::shared_ptr<ColorMap> ReadP7(std::ifstream& in, const PamHeader& header) {
  std::shared_ptr<ColorMap> img(new ColorMap(header.width, header.height));
  size_t sz = header.width * header.height;
  float maxval = (float)header.maxval;
  
  for (size_t i = 0; i < sz; ++i) {
    Color32 c32;
    
    if (header.depth == 1) {
      uint8_t c;
      in.read((char*)&c, 1);
      c32 = {c, c, c, 255};
    } else if (header.depth == 3) {
      uint8_t c[3]; 
      in.read((char*)&c, 3);
      c32 = {c[0], c[1], c[2], 255};
    } else if (header.depth == 4) {
      uint8_t c[4]; 
      in.read((char*)&c, 4);
      c32 = {c[0], c[1], c[2], c[3]};
    } else {
      throw std::runtime_error("PortablePixMap::ReadP7() - Unsupported depth " + 
                               std::to_string(header.depth));
    }
    img->At(i) = Color(c32.r/maxval, c32.g/maxval, c32.b/maxval, c32.a/maxval);
  }
  return img;

}

std::shared_ptr<ColorMap> PortablePixMap::Read(const std::string& filename) {
  std::ifstream in(filename, std::ios::in | std::ios::binary);
  if (!in.is_open()) {
    throw std::runtime_error("PortablePixMap::Read() - Cant open file " + filename);
  }

  std::string line;
  PpmHeader header;
  PamHeader pam_header;

  std::getline(in, header.version);

  if (header.version == "P3" || header.version == "P6" || 
      header.version == "P2" || header.version == "P5") {
    SkipCommentLine(in, line);
    std::stringstream dimensions(line);
    dimensions >> header.width;
    dimensions >> header.height;
    std::getline(in, line);
    std::stringstream maxval(line);
    maxval >> header.maxval;
  } else if (header.version == "P7") {
    pam_header.version = header.version;
    // Checks EOF in getline
    while (std::getline(in, line)) {
      if (!line.empty() && line[0] == '#') {
        continue;
      }
      if (line == "ENDHDR") {
        break;
      }

      // Yeah c++ and split string, easy!!!
      std::stringstream iss(line);
      std::vector<std::string> tokens{std::istream_iterator<std::string>{iss},
                                      std::istream_iterator<std::string>{}};

      if (tokens[0] == "WIDTH") {
        pam_header.width = std::stoi(tokens[1]);
      } else if (tokens[0] == "HEIGHT") {
        pam_header.height = std::stoi(tokens[1]);
      } else if (tokens[0] == "DEPTH") {
        pam_header.depth = std::stoi(tokens[1]);
      } else if (tokens[0] == "MAXVAL") {
        pam_header.maxval = std::stoi(tokens[1]);
      } else if (tokens[0] == "TUPLTYPE") {
        pam_header.tupltype = tokens[1]; 
      }

      header.maxval = pam_header.maxval;
    }
  }

  if (header.maxval > 255) {
    throw std::runtime_error("PortablePixMap::Read() - Cant load " + filename + 
                             ", version " + header.version + ". Maximum 255 colors per channel!");
  }

  if (header.version == "P6") {
    return ReadP6(in, header);
  } else if (header.version == "P5") {
    return ReadP5(in, header);
  } else if (header.version == "P3") {
    return ReadP3(in, header);
  } else if (header.version == "P7") {
    return ReadP7(in, pam_header);
  } else {
    throw std::runtime_error("PortablePixMap::Read() - Cant load " + filename + 
                             ", version " + header.version + " not supported!");
  }
}

void PortablePixMap::Write(const std::string& filename, const ColorMap& img) {
  std::ofstream out(filename, std::ios::out | std::ios::binary);
  if (!out.is_open()) {
    throw std::runtime_error("PortablePixMap::Write() - Cant open file " + filename);
  }

  out << "P6\n";
  out << img.GetWidth() << " " << img.GetHeight() << "\n";
  out << 255 << "\n";

  size_t sz = img.GetWidth() * img.GetHeight();
  for (size_t i = 0; i < sz; ++i) {
    const Color& c = img.At(i);
    uint8_t rgb[3] = {(uint8_t)Math::Clamp(0.0f, 255.0f, c.r*255),
                      (uint8_t)Math::Clamp(0.0f, 255.0f, c.g*255),
                      (uint8_t)Math::Clamp(0.0f, 255.0f, c.b*255)};
    out.write((char*)rgb, 3);
  }
}

} // namespace Image
