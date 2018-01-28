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

#ifndef _MATH_MAIN_H_5001FCDA_6109_419C_B308_1643B79E4B17_
#define _MATH_MAIN_H_5001FCDA_6109_419C_B308_1643B79E4B17_ 

#include <cmath>

//////////////////////////////////////////////////////////////////////////////
// Misc math-related functions 
//////////////////////////////////////////////////////////////////////////////
namespace Math {

inline float Lerp(float a, float b, float t) {
  return a * (1 - t) + b * t;
}

inline float InverseLerp(float min_value, float max_value, float value) {
  return (value - min_value) / (max_value - min_value);
}

inline float Random() {
  return ((float) rand() / (RAND_MAX));
}

inline float Random(float fmin, float fmax) {
  float t = Random(); 
  return Lerp(fmin, fmax, t);
}

template <typename T> T Clamp(T from, T to, T what) {
  if (what < from) what = from;
  if (what > to) what = to;
  return what;
}

template <typename T> T Sign(T value) {
  if (value <= 0) return -1;
  return 1;
}

} // namespace Math

#endif // _MATH_MAIN_H_5001FCDA_6109_419C_B308_1643B79E4B17_
