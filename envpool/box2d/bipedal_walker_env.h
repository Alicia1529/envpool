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
// https://github.com/openai/gym/blob/0.23.1/gym/envs/box2d/bipedal_walker.py

#ifndef ENVPOOL_BOX2D_BIPEDAL_WALKER_ENV_H_
#define ENVPOOL_BOX2D_BIPEDAL_WALKER_ENV_H_

#include <box2d/box2d.h>

#include <array>
#include <memory>
#include <random>
#include <vector>

namespace box2d {

class BipedalWalkerBox2dEnv {};

}  // namespace box2d

#endif  // ENVPOOL_BOX2D_BIPEDAL_WALKER_ENV_H_
