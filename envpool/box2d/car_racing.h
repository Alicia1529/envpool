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
// https://github.com/openai/gym/blob/master/gym/envs/box2d/car_racing.py

#ifndef ENVPOOL_BOX2D_CAR_RACING_H_
#define ENVPOOL_BOX2D_CAR_RACING_H_

#include <cmath>
#include <random>
#include <vector>
#include <unordered_set>
#include <box2d/box2d.h>

#include "envpool/core/async_envpool.h"
#include "envpool/core/env.h"

namespace box2d {

class CarRacingEnvFns;
class CarRacingEnv;


static float kRoadColor[3] = {0.4, 0.4, 0.4};

static float kSize = 0.02;
static float kEnginePower = 100000000 * kSize * kSize;
static float kWheelMomentOfInertia = 4000 * kSize * kSize;
static float kFrictionLimit = 1000000 * kSize * kSize;
// friction ~= mass ~= size^2 (calculated implicitly using density)
static float kWheelR = 27;
static float kWheelW = 14;
// WHEELPOS = [(-55, +80), (+55, +80), (-55, -82), (+55, -82)]
static float kWheelPos[8] = {-55, +80, +55, +80, -55, -82, +55, -82};
static float kHullPoly1[8] = {-60, +130, +60, +130, +60, +110, -60, +110};
static float kHullPoly2[8] = {-15, +120, +15, +120, +20, +20, -20, 20};
static float kHullPoly3[16] = {+25, +20, +50, -10, +50, -40, +20, -90, -20, -90, -50, -40, -50, -10, -25, +20};
static float kHullPoly4[8] = {-50, -120, +50, -120, +50, -90, -50, -90};

static float kWheelColor[3] = {0.0, 0.0, 0.0};
static float kWheelWhite[3] = {0.3, 0.3, 0.3};
static float kMudColor[3] = {0.4, 0.4, 0.0};


typedef struct UserData{
  b2Body* body;
  bool isTile;
  bool tileRoadVisited;
  float tileColor[3];
  float roadFriction;
  std::unordered_set<struct UserData*> objTiles;
} UserData;


class Particle {
  public:
    float color[3];
    float ttl;
    std::vector<std::tuple<float, float> > poly;
    bool isGrass;
    Particle(b2Vec2 point1, b2Vec2 point2, bool _isGrass) {
      if (!_isGrass) {
        memcpy(color, kWheelColor, 3 * sizeof(float));
      } else {
        memcpy(color, kMudColor, 3 * sizeof(float));
      }
      ttl = 1;
      poly.push_back({point1.x, point1.y});
      poly.push_back({point2.x, point2.y});
      isGrass = _isGrass;
    };
};

typedef struct WheelData{
  float wheel_rad;
  float color[3];
  float gas;
  float brake;
  float steer;
  float phase;
  float omega;
  b2Vec2* skid_start;
  Particle* skid_particle;
  b2RevoluteJoint *joint;
  std::unordered_set<UserData*> tiles;
  b2Body* wheel;
} WheelData;

b2PolygonShape generatePolygon(float* array, int size);

class FrictionDetector: public b2ContactListener {
  public:
    FrictionDetector(CarRacingEnv* _env);
    void BeginContact (b2Contact *contact);
    void EndContact (b2Contact *contact);
  protected:
    box2d::CarRacingEnv* env;
  private:
    void _Contact(b2Contact *contact, bool begin);

};


class Car {
  public:
    Car(b2World* _world, float init_angle, float init_x, float init_y);
    void gas(float g);
    void brake(float b);
    void steer(float s);
    void step(float dt);
  protected:
    b2World* world;
    b2Body* hull;
    std::vector<WheelData*> wheels;
    std::vector<b2Fixture*> hullFixtures;
    std::vector<Particle*> particles;
    float fuel_spent = 0.f;
    const float color[3] = {0.8, 0.0, 0.0};
    const float wheelPoly[8] = {-kWheelW, +kWheelR, +kWheelW, +kWheelR, +kWheelW, -kWheelR, -kWheelW, -kWheelR};
    Particle* createParticle(b2Vec2 point1, b2Vec2 point2, bool isGrass) {
      Particle* p = new Particle(point1, point2, isGrass);
      particles.push_back(p);
      while (particles.size() > 30) {
        particles.erase(particles.begin());
      }
      return p;
    };
};


class CarRacingEnvFns {
 public:
  static decltype(auto) DefaultConfig() {
    return MakeDict("max_episode_steps"_.bind(1000),
                    "reward_threshold"_.bind(900.0));
  }
  template <typename Config>
  static decltype(auto) StateSpec(const Config& conf) {
    return MakeDict("obs"_.bind(Spec<uint8_t>({96, 96, 3}, {0, 256})));
  }
  template <typename Config>
  static decltype(auto) ActionSpec(const Config& conf) {
    // TODO(alicia): specify range for steer, gas, brake:
    // np.array([-1, 0, 0]).astype(np.float32),
    // np.array([+1, +1, +1]).astype(np.float32),
    return MakeDict("action"_.bind(Spec<float>({3})));
  }
};

typedef class EnvSpec<CarRacingEnvFns> CarRacingEnvSpec;

class CarRacingEnv : public Env<CarRacingEnvSpec> {
 protected:
    const int stateW = 96;
    const int stateH = 96;
    const int videoW = 600;
    const int videoH = 400;
    const int windowW = 1000;
    const int windowH = 800;
    const float scale = 6.0; //Track scale
    const float trackRAD = 900 / scale; // Track is heavily morphed circle with this radius
    const float playfiled = 2000 / scale;  // Game over boundary
    const int fps = 50; // Frames per second
    const float zoom = 2.7; // Camera zoom
    const bool zoomFollow = true;  // Set to False for fixed view (don't use zoom)

    const float trackDetailStep = 21 / scale;
    const float trackTurnRate = 0.31;
    const float trackWidth = 40 / scale;
    const float border = 8 / scale;
    const int borderMinCount = 4;

    bool verbose; // true
    FrictionDetector* contactListener_keepref;
    b2World* world;

    int max_episode_steps_, elapsed_step_;
    std::uniform_real_distribution<> dist_;
    bool done_;
    
 public:
  float reward = 0.0;
  float prev_reward = 0.0;
  int tile_visited_count = 0;
  std::vector<std::vector<float>> track;

  CarRacingEnv(const Spec& spec, int env_id)
      : Env<CarRacingEnvSpec>(spec, env_id),
        max_episode_steps_(spec.config["max_episode_steps"_]),
        elapsed_step_(max_episode_steps_ + 1),
        done_(true) {
          contactListener_keepref = new FrictionDetector(this);
          b2Vec2 gravity(0.0f, 0.0f);
          world = new b2World(gravity);
          world->SetContactListener(contactListener_keepref);
        }

  bool IsDone() override { return done_; }

  void Reset() override {
    done_ = false;
    elapsed_step_ = 0;
    State state = Allocate();
    WriteObs(state, 0.0f);
  }

  void Step(const Action& action) override {
    State state = Allocate();
    WriteObs(state, 1.0f);
  }

 private:
  void WriteObs(State& state, float reward) {  // NOLINT
  }
};

typedef AsyncEnvPool<CarRacingEnv> CarRacingEnvPool;

}  // namespace box2d

#endif  // ENVPOOL_BOX2D_CarRacing_H_