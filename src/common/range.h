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

#ifndef _RANGE_H_81690775_CBD0_4E9F_8E7B_01AEFA24325D_
#define _RANGE_H_81690775_CBD0_4E9F_8E7B_01AEFA24325D_ 

template <typename T>
class Range {
 public:
  Range(T min, T max) : min_(min), max_(max), type_(kRegular) {}

  bool In(T value) const {
    if (type_ == kRegular) {
      if (value >= min_ && value <= max_) {
        return true;
      }
    } else if (type_ == kAll) {
      return true;
    } 
    // None
    return false;
  }

  static Range<T> All;
  static Range<T> None;

 private:
  enum Type {
    kRegular,
    kAll,
    kNone
  };
  
  explicit Range(Type type) : type_(type) {}

  T min_;
  T max_;
  Type type_;
};

template <typename T>
Range<T> Range<T>::All = Range<T>(Range::kAll);

template <typename T>
Range<T> Range<T>::None = Range<T>(Range::kNone);

#endif // _RANGE_H_81690775_CBD0_4E9F_8E7B_01AEFA24325D_
