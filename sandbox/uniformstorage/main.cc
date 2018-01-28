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

#include "uniformstorage.h"
#include <iostream>

int main() {
  ShaderUniformVariable variable;
  //variable.Set(1.0f);
  //variable.Set(100);
  //variable.Set(&variable);
  //variable.Set(glm::mat4(1.0f));

  ShaderUniformStorage storage;
  storage.Set("float", 1.0f);
  storage.Set("float2", glm::vec2(1.1f, 2.2f));
  storage.Set("int", 100);
  storage.Set("ptr", &variable);
  storage.Set("mat4x4", glm::mat4(1.0f));

  std::cerr << "checking... " << std::endl;
  GLenum type;
  std::cout << storage.Get("ptr", type) << ", "
            << &variable << std::endl;

  std::cout << *(float*)storage.Get("float", type) << ", ";
  std::cout <<  (type == GL_FLOAT) << std::endl;
  
  std::cout << storage.Get("mat4x4", type) << ", ";
  std::cout << (type == GL_FLOAT_MAT4) << std::endl;

  return 0;
}
