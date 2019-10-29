#ifndef DATA_FRAME_HPP_
#define DATA_FRAME_HPP_

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

//---

enum FrameType { FRAME_PLAYER = 0, FRAME_BALL, NUM_OF_FRAMETYPES };

class DataFrame {
 private:
  FrameType frame_type_;

 public:
  DataFrame(FrameType in_ft);
  ~DataFrame();

  virtual DataFrame* Clone() = 0;

  FrameType GetFrameType() const;
  virtual bool ThresholdCheck(DataFrame& in_future_df) = 0;
  virtual DataFrame* InterpolateForward(unsigned int in_dist_to_target,
                                        unsigned int in_dist_to_point_b,
                                        DataFrame& in_point_b) = 0;
};

//---

class PlayerFrame : public DataFrame {
 public:
  glm::vec3 position_;
  glm::quat rotation_;
  glm::vec3 scale_;

  // public:
  PlayerFrame();
  PlayerFrame(glm::vec3 in_pos, glm::quat in_rot, glm::vec3 in_scale);
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
  BallFrame(glm::vec3 in_pos, glm::quat in_rot, glm::vec3 in_scale);
  ~BallFrame();

  DataFrame* Clone();

  bool ThresholdCheck(DataFrame& in_future_df);
  DataFrame* InterpolateForward(unsigned int in_dist_to_target,
                                unsigned int in_dist_to_point_b,
                                DataFrame& in_point_b);
};

#endif  // DATA_FRAME_HPP_
