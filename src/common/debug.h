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
////

#ifndef _DEBUG_H_7BB63373_F345_4A3E_98C2_07B3D7BB1109_
#define _DEBUG_H_7BB63373_F345_4A3E_98C2_07B3D7BB1109_ 

#include <iomanip>

namespace Debug {

inline
float Getf(float flt, float prec=0.001) {
  if (abs(flt) < prec) return 0;
  return flt;
}

inline
void PrintMat4(char n, const glm::mat4& m) {
  auto g = [&m](int x, int y) {return Getf(m[x][y]);};
  #define PRNT(x, y) std::setprecision(3) << std::setw(6) << std::setfill(' ') << g(x, y) 

  // OpenGL/glm uses coloumn-major matrix indexation
  std::cerr      << "    | " << PRNT(0,0) << " " << PRNT(1,0) << " " << PRNT(2,0) << " " << PRNT(3,0) << " |" << std::endl;
  std::cerr << n <<  " = | " << PRNT(0,1) << " " << PRNT(1,1) << " " << PRNT(2,1) << " " << PRNT(3,1) << " |" << std::endl;
  std::cerr      << "    | " << PRNT(0,2) << " " << PRNT(1,2) << " " << PRNT(2,2) << " " << PRNT(3,2) << " |" << std::endl;
  std::cerr      << "    | " << PRNT(0,3) << " " << PRNT(1,3) << " " << PRNT(2,3) << " " << PRNT(3,3) << " |" << std::endl;
}

}// namespace Debug


#endif // _DEBUG_H_62C0D526_3148_456F_8198_3314DB2E9A8E_
