#ifndef DATA_FRAME_HPP_
#define DATA_FRAME_HPP_

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

//---

class DataFrame {
 private:
 public:
  DataFrame();
  ~DataFrame();

  // virtual bool DifferenceThreshold();
};

//---

class PlayerFrame : public DataFrame {
 protected:
  unsigned int float_data_length_;
  float* float_data_;
  // 3 : Position
  // 4 : Rotation
  // 3 : Scale

 public:
  PlayerFrame();
  PlayerFrame(glm::vec3 in_pos, glm::quat in_rot, glm::vec3 in_scale);
  ~PlayerFrame();

  bool ThresholdCheck(PlayerFrame& in_pf);
  PlayerFrame InterpolateForward(unsigned int in_dist_to_target,
                                 unsigned int in_dist_to_point_b,
                                 PlayerFrame& in_point_b);
};

//---

class BallFrame : public DataFrame {
 protected:
  unsigned int float_data_length_;
  float* float_data_;
  // 3 : Position
  // 4 : Rotation
  // 3 : Scale

 public:
  BallFrame();
  BallFrame(glm::vec3 in_pos, glm::quat in_rot, glm::vec3 in_scale);
  ~BallFrame();

  bool ThresholdCheck(BallFrame& in_bf);
  BallFrame InterpolateForward(unsigned int in_dist_to_target,
                               unsigned int in_dist_to_point_b,
                               BallFrame& in_point_b);
};

#endif  // DATA_FRAME_HPP_
