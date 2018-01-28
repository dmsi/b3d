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

#ifndef _UTIL_H_B32DF744_3672_4AE3_A5FB_9CB1F681FFA0_
#define _UTIL_H_B32DF744_3672_4AE3_A5FB_9CB1F681FFA0_ 

#include <string>
#include <fstream> 
#include <exception>

//////////////////////////////////////////////////////////////////////////////
// Reads text file line by line and returns content as a string. 
//////////////////////////////////////////////////////////////////////////////
inline std::string ReadFile(const std::string& filename) {
  std::string content;
  std::ifstream in(filename, std::ios::in);
  if (in.is_open()) {
    std::string line = "";
    while(std::getline(in, line)) {
      content += "\n" + line;
    }
    in.close();
    return content;
  } else {
    throw std::runtime_error("ReadFile() - cant read file " + filename);
  }
}

#endif // _UTIL_H_B32DF744_3672_4AE3_A5FB_9CB1F681FFA0_
