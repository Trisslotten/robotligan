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

  virtual bool ThresholdCheck(DataFrame& in_future_df)=0;
};

//---

class PlayerFrame : public DataFrame {
 protected:
  //unsigned int float_data_length_;
  //float* float_data_;
  // 3 : Position
  // 4 : Rotation
  // 3 : Scale

  glm::vec3 position_;
  glm::quat rotation_;
  glm::vec3 scale_;

 public:
  PlayerFrame();
  PlayerFrame(glm::vec3 in_pos, glm::quat in_rot, glm::vec3 in_scale);
  ~PlayerFrame();

  bool ThresholdCheck(DataFrame& in_future_df);
  PlayerFrame InterpolateForward(unsigned int in_dist_to_target,
                                 unsigned int in_dist_to_point_b,
                                 PlayerFrame& in_point_b);
};

//---

class BallFrame : public DataFrame {
 protected:
  // unsigned int float_data_length_;
  // float* float_data_;
  // 3 : Position
  // 4 : Rotation
  // 3 : Scale

  glm::vec3 position_;
  glm::quat rotation_;
  glm::vec3 scale_;

 public:
  BallFrame();
  BallFrame(glm::vec3 in_pos, glm::quat in_rot, glm::vec3 in_scale);
  ~BallFrame();

  bool ThresholdCheck(DataFrame& in_future_df);
  BallFrame InterpolateForward(unsigned int in_dist_to_target,
                               unsigned int in_dist_to_point_b,
                               BallFrame& in_point_b);
};

#endif  // DATA_FRAME_HPP_
