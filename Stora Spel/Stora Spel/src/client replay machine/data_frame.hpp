#ifndef DATA_FRAME_HPP_
#define DATA_FRAME_HPP_

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <ecs/components/player_component.hpp>
#include <shared/transform_component.hpp>

//---

// enum FrameType { FRAME_PLAYER = 0, FRAME_BALL, NUM_OF_FRAMETYPES };

class DataFrame {
 private:
  // FrameType frame_type_;

 public:
  DataFrame(/*FrameType in_ft*/);
  ~DataFrame();

  virtual DataFrame* Clone() = 0;

  // FrameType GetFrameType() const;
  virtual bool ThresholdCheck(DataFrame& in_future_df) = 0;
  virtual DataFrame* InterpolateForward(unsigned int in_dist_to_target,
                                        unsigned int in_dist_to_point_b,
                                        DataFrame& in_point_b) = 0;
};

//---

class PlayerFrame : public DataFrame {
 public:
  // Transform values
  glm::vec3 position_;
  glm::quat rotation_;
  glm::vec3 scale_;

  // Player values (for animations)
  float sprint_coeff_;
  bool sprinting_;
  bool running_;
  bool jumping_;

  // public:
  PlayerFrame();
  PlayerFrame(TransformComponent& in_transform_c, PlayerComponent& in_player_c);
  ~PlayerFrame();

  DataFrame* Clone();

  bool ThresholdCheck(DataFrame& in_future_df);
  DataFrame* InterpolateForward(unsigned int in_dist_to_target,
                                unsigned int in_dist_to_point_b,
                                DataFrame& in_point_b);
};

//---

class BallFrame : public DataFrame {
 public:
  glm::vec3 position_;
  glm::quat rotation_;
  glm::vec3 scale_;

  // public:
  BallFrame();
  BallFrame(TransformComponent& in_transform_c);
  ~BallFrame();

  DataFrame* Clone();

  bool ThresholdCheck(DataFrame& in_future_df);
  DataFrame* InterpolateForward(unsigned int in_dist_to_target,
                                unsigned int in_dist_to_point_b,
                                DataFrame& in_point_b);
};

#endif  // DATA_FRAME_HPP_
