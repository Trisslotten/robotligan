#include "data_frame.hpp"

#include <typeinfo>  //bad cast

#include <util/global_settings.hpp>

//##############################
//			DataFrame
//##############################

// Private---------------------------------------------------------------------

// Public----------------------------------------------------------------------

DataFrame::DataFrame(FrameType in_ft) { this->frame_type_ = in_ft; }

DataFrame::~DataFrame() {}

FrameType DataFrame::GetFrameType() const { return this->frame_type_; }

//##############################
//			PlayerFrame
//##############################

// Private---------------------------------------------------------------------

// Public----------------------------------------------------------------------

PlayerFrame::PlayerFrame() : DataFrame(FRAME_PLAYER) {}

PlayerFrame::PlayerFrame(glm::vec3 in_pos, glm::quat in_rot, glm::vec3 in_scale)
    : DataFrame(FRAME_PLAYER) {
  //
  this->position_ = in_pos;
  this->rotation_ = in_rot;
  this->scale_ = in_scale;
}

PlayerFrame::~PlayerFrame() {}

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

DataFrame* PlayerFrame::InterpolateForward(unsigned int in_dist_to_target,
                                           unsigned int in_dist_to_point_b,
                                           DataFrame& in_point_b) {
  // Cast the DataFrame to PlayerFrame
  try {
    PlayerFrame& point_b = dynamic_cast<PlayerFrame&>(in_point_b);
	//Skips forward if std::bad_cast

	// INTERPOLATED FRAME
    PlayerFrame* ret_frame = new PlayerFrame();

    // RATIO
    if (in_dist_to_point_b < 1) {
      // Prevent division on zero
      in_dist_to_point_b = 1;
    }
    float percentage_a = in_dist_to_target / in_dist_to_point_b;

    // INTERPOLATION
    // Take a percentage of the first (this) frame's values
    // and add them to the remaining percentage of the
    // second (given) frame's.

    // POSITION
    ret_frame->position_ =
        this->position_ + (point_b.position_ - this->position_) * percentage_a;

    // ROTATION
    ret_frame->rotation_ =
        glm::slerp(this->rotation_, point_b.rotation_, percentage_a);

    // SCALE : (NTS: Is this even needed?)
    ret_frame->scale_ =
        this->scale_ + (point_b.scale_ - this->scale_) * percentage_a;

    return ret_frame;

  } catch (std::bad_cast exp) {
    GlobalSettings::Access()->WriteError("data_frame.cpp", "PlayerFrame::InterpolateForward", "Bad cast");
    return nullptr;
  }
}

//##############################
//			BallFrame
//##############################

// Private---------------------------------------------------------------------

// Public----------------------------------------------------------------------

BallFrame::BallFrame() : DataFrame(FRAME_BALL) {}

BallFrame::BallFrame(glm::vec3 in_pos, glm::quat in_rot, glm::vec3 in_scale)
    : DataFrame(FRAME_BALL) {
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
}

DataFrame* BallFrame::InterpolateForward(unsigned int in_dist_to_target,
                              unsigned int in_dist_to_point_b,
                              DataFrame& in_point_b) {
  // Cast the DataFrame to PlayerFrame
  try {
    BallFrame& point_b = dynamic_cast<BallFrame&>(in_point_b);
    // Skips forward if std::bad_cast

    // INTERPOLATED FRAME
    BallFrame* ret_frame = new BallFrame();

    // RATIO
    if (in_dist_to_point_b < 1) {
      // Prevent division on zero
      in_dist_to_point_b = 1;
    }
    float percentage_a = in_dist_to_target / in_dist_to_point_b;

    // INTERPOLATION
    // Take a percentage of the first (this) frame's values
    // and add them to the remaining percentage of the
    // second (given) frame's.

    // POSITION
    ret_frame->position_ =
        this->position_ + (point_b.position_ - this->position_) * percentage_a;

    // ROTATION
    ret_frame->rotation_ =
        glm::slerp(this->rotation_, point_b.rotation_, percentage_a);

    // SCALE : (NTS: Is this even needed?)
    ret_frame->scale_ =
        this->scale_ + (point_b.scale_ - this->scale_) * percentage_a;

    return ret_frame;

  } catch (std::bad_cast exp) {
    GlobalSettings::Access()->WriteError(
        "data_frame.cpp", "BallFrame::InterpolateForward", "Bad cast");
    return nullptr;
  }
}
