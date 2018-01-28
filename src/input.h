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

#ifndef _INPUT_H_77DB633C_2377_4682_95CD_F8B7C011ED94_
#define _INPUT_H_77DB633C_2377_4682_95CD_F8B7C011ED94_ 

#include <string>
#include "gl_main.h"
#include "glm_main.h"

//////////////////////////////////////////////////////////////////////////////
// Input class. Will contain stuff like Get("Jump"), etc...
//////////////////////////////////////////////////////////////////////////////
class Input {
 public:
   ///////////////////////////////////////////////////////////////////////////
   // Works as GetAxisRaw in Unity3D
   // "Vertical" "Horizontal" "Mouse X" "Mouse Y"
   ///////////////////////////////////////////////////////////////////////////
   float       GetAxis                 (const std::string& axis) const;
   bool        GetMouseButtonDown      (int button)              const;
   bool        GetKey                  (int key)                 const;
   bool        GetKeyDown              (int key)                 const;
   bool        GetKeyUp                (int key)                 const;

   // This supposed to be private
   void        Update                  ();

   void        Init();

 private:
   GLFWwindow* GetWindow               ()                        const;

   glm::vec2   GetMouseDelta           ()                        const;

   glm::vec2 mouse_position_ = glm::vec2(0, 0);
   glm::vec2 old_mouse_position_ = glm::vec2(0, 0);
};

#endif // _INPUT_H_77DB633C_2377_4682_95CD_F8B7C011ED94_
