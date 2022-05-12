/*
 * Copyright 2022 Garena Online Private Limited
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
// https://github.com/openai/gym/blob/0.23.1/gym/envs/box2d/lunar_lander.py

#ifndef ENVPOOL_BOX2D_LUNAR_LANDER_H_
#define ENVPOOL_BOX2D_LUNAR_LANDER_H_

#include <box2d/box2d.h>

#include <array>
#include <random>
#include <vector>

namespace box2d {

class ContactDetector;

class LunarLanderEnv {
  const double kFPS = 50;
  const double kScale = 30.0;
  const double kMainEnginePower = 13.0;
  const double kSideEnginePower = 0.6;
  const double kInitialRandom = 1000.0;
  const double kLanderPoly[6][2] = {{-14, 17}, {-17, 0}, {-17, -10},
                                    {17, -10}, {17, 0},  {14, 17}};
  const double kLegAway = 20;
  const double kLegDown = 18;
  const double kLegW = 2;
  const double kLegH = 8;
  const double kLegSpringTorque = 40;
  const double kSideEngineHeight = 14.0;
  const double kSideEngineAway = 12.0;
  const double kViewportW = 600;
  const double kViewportH = 400;
  const int kChunks = 11;

  friend class ContactDetector;

 protected:
  int max_episode_steps_, elapsed_step_;
  float reward_;
  bool continuous_, done_;

  // box2d related
  b2World world_;
  b2Body *moon_, *lander_;
  std::vector<b2Body*> particles_;
  std::array<b2Body*, 2> legs_;
  std::array<bool, 2> ground_contact_;
  ContactDetector* listener_;
  std::uniform_real_distribution<> dist_;

 public:
  LunarLanderEnv(bool continuous, int max_episode_steps);
  void LunarLanderReset(std::mt19937* gen);
  // discrete action space: action0
  // continuous action space: action1 and action2
  void LunarLanderStep(std::mt19937* gen, int action0, float action1,
                       float action2);
};

class ContactDetector : public b2ContactListener {
  LunarLanderEnv* env_;

 public:
  explicit ContactDetector(LunarLanderEnv* env);
  void BeginContact(b2Contact* contact) override;
  void EndContact(b2Contact* contact) override;
};

}  // namespace box2d

#endif  // ENVPOOL_BOX2D_LUNAR_LANDER_H_
