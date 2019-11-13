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
                         PlayerComponent& in_player_c) {
  //
  this->position_ = in_transform_c.position;
  this->rotation_ = in_transform_c.rotation;
  // this->scale_ = in_transform_c.scale;

  //
  this->sprint_coeff_ = in_player_c.sprint_coeff;
  this->sprinting_ = in_player_c.sprinting;
  this->running_ = in_player_c.running;
  this->jumping_ = in_player_c.jumping;
}

PlayerFrame::~PlayerFrame() {}

DataFrame* PlayerFrame::Clone() {
  PlayerFrame* ret_ptr = new PlayerFrame();

  ret_ptr->position_ = this->position_;
  ret_ptr->rotation_ = this->rotation_;
  // ret_ptr->scale_ = this->scale_;

  ret_ptr->sprint_coeff_ = this->sprint_coeff_;
  ret_ptr->sprinting_ = this->sprinting_;
  ret_ptr->running_ = this->running_;
  ret_ptr->jumping_ = this->jumping_;

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
  if (this->sprinting_ != future_pf.sprinting_) {
    return true;
  }
  if (this->running_ != future_pf.running_) {
    return true;
  }
  if (this->jumping_ != future_pf.jumping_) {
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

    // SCALE : Can just be straight set as it never changes
    // ret_frame->scale_ = this->scale_;

    // SPRINT COEFFICIENT : (NTS: Should this even be interpolated? If no we do
    // not even need to save it)
    ret_frame->sprint_coeff_ =
        this->sprint_coeff_ +
        (point_b.sprint_coeff_ - this->sprint_coeff_) * percentage_a;

    // PLAYER BOOLEANS : Set it dependnat on how far we are towards the next
    // point
    if (percentage_a < 0.5) {
      ret_frame->sprinting_ = this->sprinting_;
      ret_frame->running_ = this->running_;
      ret_frame->jumping_ = this->jumping_;
    } else {
      ret_frame->sprinting_ = point_b.sprinting_;
      ret_frame->running_ = point_b.running_;
      ret_frame->jumping_ = point_b.jumping_;
    }

    return ret_frame;

  } catch (std::bad_cast exp) {
    GlobalSettings::Access()->WriteError(__FILE__, __FUNCTION__, "Bad cast");
    return nullptr;
  }
}

void PlayerFrame::WriteBack(TransformComponent& in_transform_c,
                            PlayerComponent& in_player_c) {
  in_transform_c.position = this->position_;
  in_transform_c.rotation = this->rotation_;
  // in_transform_c.scale = this->scale_;
  in_transform_c.scale = glm::vec3(0.1);

  in_player_c.sprint_coeff = this->sprint_coeff_;
  in_player_c.sprinting = this->sprinting_;
  in_player_c.running = this->running_;
  in_player_c.jumping = this->jumping_;
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
  // Cast the DataFrame to BallFrame
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

PickUpFrame::PickUpFrame() {
  position_ = glm::vec3(0.f);
  rotation_ = glm::quat();
}

PickUpFrame::PickUpFrame(TransformComponent& in_transform_c) {
  position_ = in_transform_c.position;
  rotation_ = in_transform_c.rotation;
}

PickUpFrame::~PickUpFrame() {}

DataFrame* PickUpFrame::Clone() {
  PickUpFrame* ret_ptr = new PickUpFrame();

  ret_ptr->position_ = position_;
  ret_ptr->rotation_ = rotation_;

  return ret_ptr;
}

DataFrame* PickUpFrame::InterpolateForward(unsigned int in_dist_to_target,
                                           unsigned int in_dist_to_point_b,
                                           DataFrame& in_point_b) {
  // Cast the DataFrame to PlayerFrame
  try {
    PickUpFrame& point_b = dynamic_cast<PickUpFrame&>(in_point_b);
    // Skips forward if std::bad_cast

    // INTERPOLATED FRAME
    PickUpFrame* ret_frame = new PickUpFrame();

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

  return nullptr;
}

bool PickUpFrame::ThresholdCheck(
    DataFrame& in_future_df) {  // Cast to BallFrame
  PickUpFrame& future_pf = dynamic_cast<PickUpFrame&>(in_future_df);

  float threshold = 0.0f;

  // POSITION
  float pos_diff = glm::distance(position_, future_pf.position_);
  threshold =
      GlobalSettings::Access()->ValueOf("REPLAY_THRESHOLD_PICKUP_POSITION");
  if (pos_diff > threshold) {
    // If we have moved over the threshold value away
    return true;
  }

  // ROTATION
  float rot_diff = glm::dot(rotation_, future_pf.rotation_);
  threshold =
      GlobalSettings::Access()->ValueOf("REPLAY_THRESHOLD_PICKUP_ROTATION");
  if (abs(rot_diff - 1.0f) > threshold) {
    // If we have rotated more than the theshhold value allows
    return true;
  }

  return false;
}

void PickUpFrame::WriteBack(TransformComponent& in_transform_c) {
  in_transform_c.position = this->position_;
  in_transform_c.rotation = this->rotation_;
  // in_transform_c.scale = this->scale_;
  in_transform_c.scale = glm::vec3(0.4f);
}

WallFrame::WallFrame() {
  position_ = glm::vec3(0.f);
  rotation_ = glm::quat();
}

WallFrame::WallFrame(TransformComponent& trans_c) {
  position_ = trans_c.position;
  rotation_ = trans_c.rotation;
}

WallFrame::~WallFrame() {}

DataFrame* WallFrame::Clone() {
  WallFrame* return_wall = new WallFrame();
  return_wall->position_ = position_;
  return_wall->rotation_ = rotation_;

  return return_wall;
}

DataFrame* WallFrame::InterpolateForward(unsigned int in_dist_to_target,
                                         unsigned int in_dist_to_point_b,
                                         DataFrame& in_point_b) {
  // Cast the DataFrame to WallFrame
  try {
    WallFrame& point_b = dynamic_cast<WallFrame&>(in_point_b);
    // Skips forward if std::bad_cast

    // INTERPOLATED FRAME
    WallFrame* ret_frame = new WallFrame();

    // RATIO
    if (in_dist_to_point_b < 1) {
      // Prevent division on zero
      in_dist_to_point_b = 1;
    }
    float percentage_a = in_dist_to_target / in_dist_to_point_b;

    // INTERPOLATION

    // POSITION
    ret_frame->position_ =
        position_ + (point_b.position_ - position_) * percentage_a;

    // ROTATION
    ret_frame->rotation_ = rotation_;

    return ret_frame;

  } catch (std::bad_cast exp) {
    GlobalSettings::Access()->WriteError(__FILE__, __FUNCTION__, "Bad cast");
    return nullptr;
  }
}

bool WallFrame::ThresholdCheck(DataFrame& in_future_df) {
  // Cast to WallFrame
  WallFrame& future_pf = dynamic_cast<WallFrame&>(in_future_df);

  float threshold = 0.0f;

  // POSITION
  float pos_diff = glm::distance(position_, future_pf.position_);
  threshold =
      GlobalSettings::Access()->ValueOf("REPLAY_THRESHOLD_WALL_POSITION");
  if (pos_diff > threshold) {
    // If we have moved over the threshold value away
    return true;
  }

  return false;
}

void WallFrame::WriteBack(TransformComponent& trans_c) {
  trans_c.position = position_;
  trans_c.rotation = rotation_;
  trans_c.scale =
      glm::vec3(1.f, 4.f, 5.f);  // Values from Playstate -> CreateWall
}

//##############################
//			ShotFrame
//##############################

// Private---------------------------------------------------------------------

// Public----------------------------------------------------------------------

ShotFrame::ShotFrame() {}

ShotFrame::ShotFrame(TransformComponent& in_transform_c) {
  //
  this->position_ = in_transform_c.position;
  this->rotation_ = in_transform_c.rotation;
  // this->scale_ = in_transform_c.scale;
}

ShotFrame::~ShotFrame() {}

ShotFrame* ShotFrame::Clone() {
  ShotFrame* ret_ptr = new ShotFrame();

  ret_ptr->position_ = this->position_;
  ret_ptr->rotation_ = this->rotation_;
  // ret_ptr->scale_ = this->scale_;

  return ret_ptr;
}

bool ShotFrame::ThresholdCheck(DataFrame& in_future_df) { return false; }

DataFrame* ShotFrame::InterpolateForward(unsigned int in_dist_to_target,
                                         unsigned int in_dist_to_point_b,
                                         DataFrame& in_point_b) {
  // Cast the DataFrame to ShotFrame
  try {
    ShotFrame& point_b = dynamic_cast<ShotFrame&>(in_point_b);
    // Skips forward if std::bad_cast

    // INTERPOLATED FRAME
    ShotFrame* ret_frame = new ShotFrame();

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

void ShotFrame::WriteBack(TransformComponent& in_transform_c) {
  in_transform_c.position = this->position_;
  in_transform_c.rotation = this->rotation_;
  // in_transform_c.scale = this->scale_;
  in_transform_c.scale = glm::vec3(0.5f);
}

//##############################
//			TeleportShotFrame
//##############################

// Private---------------------------------------------------------------------

// Public----------------------------------------------------------------------

TeleportShotFrame::TeleportShotFrame() {}

TeleportShotFrame::TeleportShotFrame(TransformComponent& in_transform_c) {
  this->position_ = in_transform_c.position;
}

TeleportShotFrame::~TeleportShotFrame() {}

TeleportShotFrame* TeleportShotFrame::Clone() {
  TeleportShotFrame* ret_ptr = new TeleportShotFrame();

  ret_ptr->position_ = this->position_;

  return ret_ptr;
}

bool TeleportShotFrame::ThresholdCheck(DataFrame& in_future_df) {
  return false;
}

DataFrame* TeleportShotFrame::InterpolateForward(
    unsigned int in_dist_to_target, unsigned int in_dist_to_point_b,
    DataFrame& in_point_b) {
  // Cast the DataFrame to ShotFrame
  try {
    TeleportShotFrame& point_b = dynamic_cast<TeleportShotFrame&>(in_point_b);
    // Skips forward if std::bad_cast

    // INTERPOLATED FRAME
    TeleportShotFrame* ret_frame = new TeleportShotFrame();

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

    return ret_frame;

  } catch (std::bad_cast exp) {
    GlobalSettings::Access()->WriteError(__FILE__, __FUNCTION__, "Bad cast");
    return nullptr;
  }
}

void TeleportShotFrame::WriteBack(TransformComponent& in_transform_c) {
  in_transform_c.position = this->position_;
  in_transform_c.rotation = glm::quat();
  in_transform_c.scale = glm::vec3(1.0f);
}
