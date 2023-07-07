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

#ifndef _POSTPROCESSPIPELINE_H_DC211483_58E1_4A91_A998_609D56C6DF59_
#define _POSTPROCESSPIPELINE_H_DC211483_58E1_4A91_A998_609D56C6DF59_ 

////////////////////////////////////////////////////////////////////////////
// Postprocessing pipeline. 
// Takes multiple inputs and provide multiple color / depth inputs.
// Between the stages uses only 0 color attachment for I/O.
// I[...] -> A -> B -> C -> .... -> Final[...]
//
// Usage:
//  using TexLs = std::vector<std::shared_ptr<Texture>>;
//  PostprocessPipeline blur(scene, "assets/postprocessing");
//  blur.Input(10, TexLs{input_tex1, input_tex2, ... });
//  blur
//    . Stage("downscale", w/8, h/8, "bypass.mat");
//  blur
//    . Stage("hblur", w/8, h/8, "gauss_blur.mat")
//    . Action<BlurUni>(w/8, h/8, vec2(1, 0));
//  blur
//    . Stage("vblur", w/8, h/8, "gauss_blur.mat")
//    . Action<BlurUni>(w/8, h/8, vec2(0, 1));
//  blur.Done();
//
//  (...)
//
//  auto final_tex = blur.Tex();  // output of the final stage
//  auto first_tex = blur.Tex(0); // output of the first stage
//
// It creates uniq names for tags out of stage names in order to prevent 
// conflicts with other pipelines if they're using same stage name.
////////////////////////////////////////////////////////////////////////////
class PostprocessPipeline {
 public:
  enum {kFinal = -1};
  using TexLs = std::vector<std::shared_ptr<Texture>>;
  using TagLs = std::vector<std::string>;

  explicit PostprocessPipeline(Scene& scene, 
                               const std::string& mtrl_path = "") 
    : scene_(scene), mtrl_path_(mtrl_path) {}

  ////////////////////////////////////////////////////////////////////////////
  // Getting stages/final results of the pipeline 
  ////////////////////////////////////////////////////////////////////////////

  std::shared_ptr<Texture> Tex(int stage = kFinal, int layer = 0) {
    if (stage == kFinal) stage = rt_ls_.size() - 1;
    return Fb(stage)->GetLayerAsTexture(layer, Layer::kColor);
  }

  std::shared_ptr<Texture> Depth(int stage = kFinal) {
    if (stage == kFinal) stage = rt_ls_.size() - 1;
    return Fb(stage)->GetLayerAsTexture(0, Layer::kDepth);
  }

  ////////////////////////////////////////////////////////////////////////////
  // Configuration
  ////////////////////////////////////////////////////////////////////////////

  void Input(int index, const TexLs& tex_ls) {
    assert(input_.empty());
    input_ = tex_ls;
    start_idx_ = index;
  }

  // Returns Cfg<Actor> reference in order to apply action
  auto& Stage(std::string tag, int w, int h, std::string mtrl) { 
    auto it = alltags_.find(tag);
    if (it != alltags_.end()) {
      ABORT_F("Tag %s already exists!", tag.c_str());
    }
    alltags_.emplace(tag);
    auto idx = start_idx_ + stage_ls_.size();
    tag = TgName(tag);
    stage_ls_.emplace_back(
        StageInfo {
        tag, w, h, MtName(mtrl),
        Cfg<RenderTarget>(scene_, RtName(idx, tag), idx),
        Cfg<Actor>       (scene_, AcName(idx, tag))
        });
    return stage_ls_.back().dcfg;
  }

  template <typename T, typename... TArgs>
  void Action(const std::string& tag, TArgs&&... args) {
    Find(TgName(tag)).dcfg.template Action<T>(std::forward<TArgs>(args)...);
  }

  // For reconfiguration if necessary
  Cfg<RenderTarget>& rcfg(const std::string& tag) {
    return Find(TgName(tag)).rcfg;
  }

  Cfg<Actor>& dcfg(const std::string& tag) {
    return Find(TgName(tag)).dcfg;
  }

  void Done() {
    assert(!input_.empty());
    assert(!stage_ls_.empty());
    // python like enumerate? for range-based loop? 
    for (int i = 0; i < stage_ls_.size(); i++) {
      auto& s = stage_ls_[i];

      // RT
      s.rcfg . Tags       (s.tag)
             . Type       (FrameBuffer::kTexture2D)
             . Resolution (s.w, s.h)
             . Layer      (Layer::kColor, Layer::kReadWrite) 
             . Layer      (Layer::kDepth, Layer::kWrite)    
             . Clear      (.4, .4, .4, 1);
      s.rcfg . Done();
      rt_ls_.emplace_back(s.rcfg.GetClient());

      s.dcfg . Model("assets/models/screen.dsm", s.mtrl)
             . Tags (0, TagLs{s.tag});

      // Display
      auto input = GetInput(i);
      for (int slot = 0; slot < input.size(); slot++) {
        s.dcfg.Texture(slot, input[slot]);
      }
      s.dcfg . Done();
    }
  }

 private:
  struct StageInfo {
    std::string tag;
    int w, h;
    std::string mtrl;
    Cfg<RenderTarget> rcfg; 
    Cfg<Actor>        dcfg;
  };

  using StageLs = std::vector<StageInfo>;
  using FbPtr = std::shared_ptr<FrameBuffer>;
  using RtPtr = std::shared_ptr<RenderTarget>;
  using RtLs = std::vector<RtPtr>;

  inline RtPtr Rt(int stage) const {
    return rt_ls_.at(stage);
  }

  inline FbPtr Fb(int stage) const {
    auto fb = Rt(stage)->GetFrameBuffer();
    assert(fb);
    return fb;
  }

  inline StageInfo& Find(const std::string& tag) {
    auto it = std::find_if(
        stage_ls_.begin(), 
        stage_ls_.end(),
        [this,&tag](const StageInfo& s) {return s.tag == tag;});
    assert(it != stage_ls_.end()); 
    return *it;
  }

  inline std::string RtName(int idx, const std::string& tag) const {
    return "rt." + std::to_string(idx) + "." + tag;
  }

  inline std::string AcName(int idx, const std::string& tag) const {
    return "actor.display." + std::to_string(idx) + "." + tag;
  }

  inline std::string TgName(const std::string& tag) const {
    return "pp." + std::to_string(start_idx_) + "." + tag;
  }

  inline std::string MtName(const std::string& mat) const {
    if (!mtrl_path_.empty()) {
      return mtrl_path_ + "/" + mat;
    }
    return mat;
  }
  
  TexLs GetInput(int stage) {
    assert(stage >= 0);
    if (stage == 0) {
      return input_;
    }
    return TexLs{Fb(stage-1)->GetLayerAsTexture(0, Layer::kColor)};
  }

  Scene&      scene_;
  std::string mtrl_path_;
  RtLs        rt_ls_;
  TexLs       input_;
  StageLs     stage_ls_;
  int         start_idx_ = 0;
  std::set<std::string> alltags_;
};
#endif // _POSTPROCESSPIPELINE_H_DC211483_58E1_4A91_A998_609D56C6DF59_
