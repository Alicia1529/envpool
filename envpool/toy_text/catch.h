/*
 * Copyright 2021 Garena Online Private Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
// https://github.com/deepmind/bsuite/blob/master/bsuite/environments/catch.py

#ifndef ENVPOOL_TOY_TEXT_CATCH_H_
#define ENVPOOL_TOY_TEXT_CATCH_H_

#include <cmath>
#include <random>

#include "envpool/core/async_envpool.h"
#include "envpool/core/env.h"

namespace toy_text {

class CatchEnvFns {
 public:
  static decltype(auto) DefaultConfig() {
    return MakeDict("height"_.Bind(10), "width"_.Bind(5));
  }
  template <typename Config>
  static decltype(auto) StateSpec(const Config& conf) {
    return MakeDict("obs"_.Bind(
        Spec<float>({conf["height"_], conf["width"_]}, {0.0F, 1.0F})));
  }
  template <typename Config>
  static decltype(auto) ActionSpec(const Config& conf) {
    return MakeDict("action"_.Bind(Spec<int>({-1}, {0, 2})));
  }
};

using CatchEnvSpec = EnvSpec<CatchEnvFns>;

class CatchEnv : public Env<CatchEnvSpec> {
 protected:
  int x_, y_, height_, width_, paddle_;
  std::uniform_int_distribution<> dist_;
  bool done_;

 public:
  CatchEnv(const Spec& spec, int env_id)
      : Env<CatchEnvSpec>(spec, env_id),
        height_(spec.config["height"_]),
        width_(spec.config["width"_]),
        dist_(0, width_ - 1),
        done_(true) {}

  bool IsDone() override { return done_; }

  void Reset() override {
    x_ = 0;
    y_ = dist_(gen_);
    paddle_ = width_ / 2;
    done_ = false;
    WriteState(0.0);
  }

  void Step(const Action& action) override {
    int act = action["action"_];
    float reward = 0.0;
    paddle_ += act - 1;
    if (paddle_ < 0) {
      paddle_ = 0;
    }
    if (paddle_ >= width_) {
      paddle_ = width_ - 1;
    }
    if (++x_ == height_ - 1) {
      done_ = true;
      reward = y_ == paddle_ ? 1.0 : -1.0;
    }
    WriteState(reward);
  }

 private:
  void WriteState(float reward) {
    State state = Allocate();
    state["obs"_](x_, y_) = 1.0F;
    state["obs"_](height_ - 1, paddle_) = 1.0F;
    state["reward"_] = reward;
  }
};

using CatchEnvPool = AsyncEnvPool<CatchEnv>;

}  // namespace toy_text

#endif  // ENVPOOL_TOY_TEXT_CATCH_H_
