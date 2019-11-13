#ifndef DATA_FRAME_HPP_
#define DATA_FRAME_HPP_

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <ecs/components.hpp>
#include <ecs/components/player_component.hpp>
#include <shared/transform_component.hpp>

#include <glob/graphics.hpp>
//---

class DataFrame {
 private:
 public:
  DataFrame();
  ~DataFrame();

  virtual DataFrame* Clone() = 0;

  virtual bool ThresholdCheck(DataFrame& in_future_df) = 0;
  virtual DataFrame* InterpolateForward(unsigned int in_dist_to_target,
                                        unsigned int in_dist_to_point_b,
                                        DataFrame& in_point_b) = 0;
};

//---

class PlayerFrame : public DataFrame {
 protected:
  // Transform values
  glm::vec3 position_;
  glm::quat rotation_;
  glm::vec3 scale_;

  // Player values (for animations)
  PlayerComponent player_c_;

  // physics stuff
  glm::vec3 velocity_;

 public:
  PlayerFrame();
  PlayerFrame(TransformComponent& in_transform_c, PlayerComponent& in_player_c,
              PhysicsComponent& in_phys_c);
  ~PlayerFrame();

  DataFrame* Clone();

  bool ThresholdCheck(DataFrame& in_future_df);
  DataFrame* InterpolateForward(unsigned int in_dist_to_target,
                                unsigned int in_dist_to_point_b,
                                DataFrame& in_point_b);
  void WriteBack(TransformComponent& in_transform_c,
                 PlayerComponent& in_player_c, PhysicsComponent& in_phys_c);
};

//---

class BallFrame : public DataFrame {
 protected:
  glm::vec3 position_;
  glm::quat rotation_;
  // glm::vec3 scale_;

  // NTS: See comment in declaration of PlayerFrame

 public:
  BallFrame();
  BallFrame(TransformComponent& in_transform_c);
  ~BallFrame();

  DataFrame* Clone();

  bool ThresholdCheck(DataFrame& in_future_df);
  DataFrame* InterpolateForward(unsigned int in_dist_to_target,
                                unsigned int in_dist_to_point_b,
                                DataFrame& in_point_b);
  void WriteBack(TransformComponent& in_transform_c);
};

#endif  // DATA_FRAME_HPP_
