#include "data_frame.hpp"

#include <util/global_settings.hpp>

//##############################
//			DataFrame
//##############################

// Private---------------------------------------------------------------------

// Public----------------------------------------------------------------------

DataFrame::DataFrame() {}

DataFrame::~DataFrame() {}

//##############################
//			PlayerFrame
//##############################

// Private---------------------------------------------------------------------

// Public----------------------------------------------------------------------

PlayerFrame::PlayerFrame() {
  // this->float_data_length_ = 0;
  // this->float_data_ = nullptr;
}

PlayerFrame::PlayerFrame(glm::vec3 in_pos, glm::quat in_rot,
                         glm::vec3 in_scale) {
  // Number of floats required
  // this->float_data_length_ = 3 + 4 + 3;
  //
  // Allocate space
  // this->float_data_ = new float[this->float_data_length_];

  // Fill space
  // this->float_data_[0] = in_pos.x;
  // this->float_data_[1] = in_pos.y;
  // this->float_data_[2] = in_pos.z;
  // this->float_data_[3] = in_rot.x;
  // this->float_data_[4] = in_rot.y;
  // this->float_data_[5] = in_rot.z;
  // this->float_data_[6] = in_rot.w;
  // this->float_data_[7] = in_pos.x;
  // this->float_data_[8] = in_pos.y;
  // this->float_data_[9] = in_pos.z;

  //
  this->position_ = in_pos;
  this->rotation_ = in_rot;
  this->scale_ = in_scale;
}

PlayerFrame::~PlayerFrame() {
  // if (this->float_data_ != nullptr) {
  //  delete[] this->float_data_;
  //}
}

bool PlayerFrame::ThresholdCheck(DataFrame& in_future_df) {
  // Cast to PlayerFrame
  PlayerFrame& future_pf = dynamic_cast<PlayerFrame&>(in_future_df);

  float threshold = 0.0f;

  // POSITION
  float pos_diff = glm::distance(this->position_, future_pf.position_);
  threshold =
      GlobalSettings::Access()->ValueOf("REPLAY_THRESHOLD_PLAYER_POSITION");
  if (pos_diff > threshold) {
    // If we have moved over the threshold value away
    return true;
  }

  // ROTATION
  float rot_diff = glm::dot(this->rotation_, future_pf.rotation_);
  threshold =
      GlobalSettings::Access()->ValueOf("REPLAY_THRESHOLD_PLAYER_ROTATION");
  if (abs(rot_diff - 1.0f) > threshold) {
    // If we have rotated more than the theshhold value allows
    return true;
  }

  return false;
}

PlayerFrame PlayerFrame::InterpolateForward(unsigned int in_dist_to_target,
                                            unsigned int in_dist_to_point_b,
                                            PlayerFrame& in_point_b) {
  // INTERPOLATED FRAME
  PlayerFrame ret_frame;

  // RATIO
  float percentage_a = in_dist_to_target / in_dist_to_point_b;
  // float percentage_b = (1 - percentage_a);

  // INTERPOLATION
  // Take a percentage of the first (this) frame's values
  // and add them to the remaining percentage of the
  // second (given) frame's.

  // POSITION
  ret_frame.position_ =
      this->position_ + (in_point_b.position_ - this->position_) * percentage_a;

  // ROTATION
  ret_frame.rotation_ =
      glm::slerp(this->rotation_, in_point_b.rotation_, percentage_a);

  // SCALE : (NTS: Is this even needed?)
  ret_frame.scale_ =
      this->scale_ + (in_point_b.scale_ - this->scale_) * percentage_a;

  return ret_frame;
}

//##############################
//			BallFrame
//##############################

// Private---------------------------------------------------------------------

// Public----------------------------------------------------------------------

BallFrame::BallFrame() {}

BallFrame::BallFrame(glm::vec3 in_pos, glm::quat in_rot, glm::vec3 in_scale) {
  //
  this->position_ = in_pos;
  this->rotation_ = in_rot;
  this->scale_ = in_scale;
}

BallFrame::~BallFrame() {}

bool BallFrame::ThresholdCheck(DataFrame& in_future_df) {
  // Cast to BallFrame
  BallFrame& future_pf = dynamic_cast<BallFrame&>(in_future_df);

  float threshold = 0.0f;

  // POSITION
  float pos_diff = glm::distance(this->position_, future_pf.position_);
  threshold =
      GlobalSettings::Access()->ValueOf("REPLAY_THRESHOLD_BALL_POSITION");
  if (pos_diff > threshold) {
    // If we have moved over the threshold value away
    return true;
  }

  // ROTATION
  float rot_diff = glm::dot(this->rotation_, future_pf.rotation_);
  threshold =
      GlobalSettings::Access()->ValueOf("REPLAY_THRESHOLD_BALL_ROTATION");
  if (abs(rot_diff - 1.0f) > threshold) {
    // If we have rotated more than the theshhold value allows
    return true;
  }

  return false;
  return false;
}

BallFrame BallFrame::InterpolateForward(unsigned int in_dist_to_target,
                                        unsigned int in_dist_to_point_b,
                                        BallFrame& in_point_b) {
  // INTERPOLATED FRAME
  BallFrame ret_frame;

  // RATIO
  float percentage_a = in_dist_to_target / in_dist_to_point_b;
  // float percentage_b = (1 - percentage_a);

  // INTERPOLATION
  // Take a percentage of the first (this) frame's values
  // and add them to the remaining percentage of the
  // second (given) frame's.

  // POSITION
  ret_frame.position_ =
      this->position_ + (in_point_b.position_ - this->position_) * percentage_a;

  // ROTATION
  ret_frame.rotation_ =
      glm::slerp(this->rotation_, in_point_b.rotation_, percentage_a);

  // SCALE : (NTS: Is this even needed?)
  ret_frame.scale_ =
      this->scale_ + (in_point_b.scale_ - this->scale_) * percentage_a;

  return ret_frame;
}
