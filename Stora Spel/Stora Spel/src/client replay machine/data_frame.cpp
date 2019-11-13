#include "data_frame.hpp"

#include <typeinfo>  //bad cast

#include <util/global_settings.hpp>

#include <iostream>  //TEMP

//##############################
//			DataFrame
//##############################

// Private---------------------------------------------------------------------

// Public----------------------------------------------------------------------

DataFrame::DataFrame() {}

DataFrame::~DataFrame() {}

// FrameType DataFrame::GetFrameType() const { return this->frame_type_; }

//##############################
//			PlayerFrame
//##############################

// Private---------------------------------------------------------------------

// Public----------------------------------------------------------------------

PlayerFrame::PlayerFrame() {}

PlayerFrame::PlayerFrame(TransformComponent& in_transform_c,
                         PlayerComponent& in_player_c_,
                         PhysicsComponent& in_phys_c) {
  //
  this->position_ = in_transform_c.position;
  this->rotation_ = in_transform_c.rotation;
  this->scale_ = in_transform_c.scale;

  player_c_ = in_player_c_;

  /*for (auto anim : in_anim_c.active_animations) {
    active_animations_.push_back(anim);
  }*/

  // physic stuff
  this->velocity_ = in_phys_c.velocity;
}

PlayerFrame::~PlayerFrame() {}

DataFrame* PlayerFrame::Clone() {
  PlayerFrame* ret_ptr = new PlayerFrame();

  ret_ptr->position_ = this->position_;
  ret_ptr->rotation_ = this->rotation_;
  ret_ptr->scale_ = this->scale_;

  ret_ptr->player_c_ = this->player_c_;

  ret_ptr->velocity_ = this->velocity_;

  return ret_ptr;
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

  // ANIMATIONS
  if (this->player_c_.sprinting != future_pf.player_c_.sprinting) {
    return true;
  }
  if (this->player_c_.running != future_pf.player_c_.running) {
    return true;
  }
  if (this->player_c_.jumping != future_pf.player_c_.jumping) {
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
    // Skips forward if std::bad_cast

    // INTERPOLATED FRAME
    PlayerFrame* ret_frame = new PlayerFrame();

    // RATIO
    if (in_dist_to_point_b < 1) {
      // Prevent division on zero
      in_dist_to_point_b = 1;
    }
    float percentage_a = in_dist_to_target / in_dist_to_point_b;

    // INTERPOLATION
    // vvv

    // POSITION
    ret_frame->position_ =
        this->position_ + (point_b.position_ - this->position_) * percentage_a;

    // ROTATION
    ret_frame->rotation_ =
        glm::slerp(this->rotation_, point_b.rotation_, percentage_a);

    ret_frame->scale_ = this->scale_;

    // PLAYER COMPONENT : Set it dependnat on how far we are towards the next
    // point
    if (percentage_a < 0.5) {
      ret_frame->velocity_ = this->velocity_;
      ret_frame->player_c_ = this->player_c_;
    } else {
      ret_frame->velocity_ = point_b.velocity_;
      ret_frame->player_c_ = point_b.player_c_;
    }

    return ret_frame;

  } catch (std::bad_cast exp) {
    GlobalSettings::Access()->WriteError(__FILE__, __FUNCTION__, "Bad cast");
    return nullptr;
  }
}

void PlayerFrame::WriteBack(TransformComponent& in_transform_c,
                            PlayerComponent& in_player_c_,
                            PhysicsComponent& in_phys_c) {
  in_transform_c.position = this->position_;
  in_transform_c.rotation = this->rotation_;
  in_transform_c.scale = this->scale_;

  in_player_c_ = player_c_;
  in_phys_c.velocity = velocity_;
}

//##############################
//			BallFrame
//##############################

// Private---------------------------------------------------------------------

// Public----------------------------------------------------------------------

BallFrame::BallFrame() {}

BallFrame::BallFrame(TransformComponent& in_transform_c) {
  //
  this->position_ = in_transform_c.position;
  this->rotation_ = in_transform_c.rotation;
  // this->scale_ = in_transform_c.scale;
}

BallFrame::~BallFrame() {}

DataFrame* BallFrame::Clone() {
  BallFrame* ret_ptr = new BallFrame();

  ret_ptr->position_ = this->position_;
  ret_ptr->rotation_ = this->rotation_;
  // ret_ptr->scale_ = this->scale_;

  return ret_ptr;
}

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
    // vvv

    // POSITION
    ret_frame->position_ =
        this->position_ + (point_b.position_ - this->position_) * percentage_a;

    // ROTATION
    ret_frame->rotation_ =
        glm::slerp(this->rotation_, point_b.rotation_, percentage_a);

    // SCALE
    // ret_frame->scale_ = this->scale_;

    return ret_frame;

  } catch (std::bad_cast exp) {
    GlobalSettings::Access()->WriteError(__FILE__, __FUNCTION__, "Bad cast");
    return nullptr;
  }
}

void BallFrame::WriteBack(TransformComponent& in_transform_c) {
  in_transform_c.position = this->position_;
  in_transform_c.rotation = this->rotation_;
  // in_transform_c.scale = this->scale_;
  in_transform_c.scale = glm::vec3(1.0);
}
