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

#ifndef _TAGS_H_C80FF947_FD26_489F_8410_87440552BF6E_
#define _TAGS_H_C80FF947_FD26_489F_8410_87440552BF6E_ 

#include <functional>
#include <vector>
#include <string>

////////////////////////////////////////////////////////////////////////////
// Very simple tags... hash tags
////////////////////////////////////////////////////////////////////////////
class Tags {
 public:
  explicit Tags(const std::vector<std::string>& tags = {}) {
    Set(tags);
  }

  void Set(const std::vector<std::string>& tags) {
    tags_.clear();
    for(const auto& t: tags) {
      auto hash = std::hash<std::string>()(t);
      tags_.emplace(hash);
    }
  }

  bool Check(const Tags& other) const {
    for (auto t: other.tags_) {
      if (Check(t)) {
        return true;
      }
    }
    return false;
  }

  bool Check(size_t tag) const {
    return tags_.find(tag) != tags_.end();
  }

 private:
  std::set<size_t> tags_;
};

#endif // _TAGS_H_C80FF947_FD26_489F_8410_87440552BF6E_
