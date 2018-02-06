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

#ifndef _ARRAY2D_H_E736CA99_9A13_400E_ADAF_30C6A3E7EABF_
#define _ARRAY2D_H_E736CA99_9A13_400E_ADAF_30C6A3E7EABF_ 

#include <cstddef>
#include <vector>

//////////////////////////////////////////////////////////////////////////////
// Simple home-made 2D array with .At() interface. 
//////////////////////////////////////////////////////////////////////////////
template <class T>
class Array2d {
 public:
  Array2d(size_t width, size_t height) 
    : width_(width), 
      height_(height),
      array_(width * height) {
  }

  void Resize(size_t width, size_t height) {
    array_.resize(width * height);
  }

  T& At(size_t index) {
    return array_.at(index);
  }

  const T& At(size_t index) const {
    return array_.at(index);
  }
  
  T& At(size_t x, size_t y) {
    return At(Index(x, y));
  }

  const T& At(size_t x, size_t y) const {
    return At(Index(x, y));
  }

  size_t GetWidth() const { return width_; }
  size_t GetHeight() const { return height_; }
  size_t GetLength() const { return width_ * height_; }
  const T* GetArray() const { return &array_.at(0); }

 private:
  size_t Index(size_t x, size_t y) const {
    return x + y * width_; 
  }
  
  size_t          width_;
  size_t          height_;
  std::vector<T>  array_;
};


#endif // _ARRAY2D_H_E736CA99_9A13_400E_ADAF_30C6A3E7EABF_
