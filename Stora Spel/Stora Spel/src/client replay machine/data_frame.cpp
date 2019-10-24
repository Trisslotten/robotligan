#include "data_frame.hpp"

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
  this->float_data_length_ = 0;
  this->float_data_ = nullptr;
}

PlayerFrame::PlayerFrame(glm::vec3 in_pos, glm::quat in_rot,
                         glm::vec3 in_scale) {
  //Number of floats required
  this->float_data_length_ = 3 + 4 + 3;

  //Allocate space
  this->float_data_ = new float[this->float_data_length_];

  //Fill space
  this->float_data_[0] = in_pos.x;
  this->float_data_[1] = in_pos.y;
  this->float_data_[2] = in_pos.z;
  this->float_data_[3] = in_rot.x;
  this->float_data_[4] = in_rot.y;
  this->float_data_[5] = in_rot.z;
  this->float_data_[6] = in_rot.w;
  this->float_data_[7] = in_pos.x;
  this->float_data_[8] = in_pos.y;
  this->float_data_[9] = in_pos.z;
}

PlayerFrame::~PlayerFrame() {
  if (this->float_data_ != nullptr) {
    delete[] this->float_data_;
  }
}

bool PlayerFrame::ThresholdCheck(PlayerFrame& in_pf) { return false; }

PlayerFrame PlayerFrame::InterpolateForward(unsigned int in_dist_to_target,
                                            unsigned int in_dist_to_point_b,
                                            PlayerFrame& in_point_b) {
  // INTERPOLATED FRAME
  PlayerFrame ret_frame;

  // RATIO
  float percentage_a = in_dist_to_target / in_dist_to_point_b;
  float percentage_b = (1 - percentage_a);

  // INTERPOLATION
  // Take a percentage of the first (this) frame's values
  // and add them to the remaining percentage of the
  // second (given) frame's.
  
  // Float data
  for (unsigned int i = 0; i < this->float_data_length_; i++) {
    ret_frame.float_data_[i] = percentage_a * this->float_data_[i] +
                               percentage_b * in_point_b.float_data_[i];
  }

  return ret_frame;
}

//##############################
//			BallFrame
//##############################

// Private---------------------------------------------------------------------

// Public----------------------------------------------------------------------

BallFrame::BallFrame() {
  this->float_data_length_ = 0;
  this->float_data_ = nullptr;
}

BallFrame::BallFrame(glm::vec3 in_pos, glm::quat in_rot, glm::vec3 in_scale) {
  // Number of floats required
  this->float_data_length_ = 3 + 4 + 3;

  // Allocate space
  this->float_data_ = new float[this->float_data_length_];

  // Fill space
  this->float_data_[0] = in_pos.x;
  this->float_data_[1] = in_pos.y;
  this->float_data_[2] = in_pos.z;
  this->float_data_[3] = in_rot.x;
  this->float_data_[4] = in_rot.y;
  this->float_data_[5] = in_rot.z;
  this->float_data_[6] = in_rot.w;
  this->float_data_[7] = in_pos.x;
  this->float_data_[8] = in_pos.y;
  this->float_data_[9] = in_pos.z;
}

BallFrame::~BallFrame() {
  if (this->float_data_ != nullptr) {
    delete[] this->float_data_;
  }
}

bool BallFrame::ThresholdCheck(BallFrame& in_bf) { return false; }

BallFrame BallFrame::InterpolateForward(unsigned int in_dist_to_target,
                                        unsigned int in_dist_to_point_b,
                                        BallFrame& in_point_b) {
  // INTERPOLATED FRAME
  BallFrame ret_frame;

  // RATIO
  float percentage_a = in_dist_to_target / in_dist_to_point_b;
  float percentage_b = (1 - percentage_a);

  // INTERPOLATION
  // Take a percentage of the first (this) frame's values
  // and add them to the remaining percentage of the
  // second (given) frame's.

  // Float data
  for (unsigned int i = 0; i < this->float_data_length_; i++) {
    ret_frame.float_data_[i] = percentage_a * this->float_data_[i] +
                               percentage_b * in_point_b.float_data_[i];
  }

  return ret_frame;
}
