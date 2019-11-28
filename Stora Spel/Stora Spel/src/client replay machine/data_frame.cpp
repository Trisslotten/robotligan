#include "data_frame.hpp"

#include <typeinfo>  //bad cast

#include <util/global_settings.hpp>

#include <iostream>  //TEMP

//##############################
//			DataFrame
//##############################

DataFrame::DataFrame() {}

DataFrame::~DataFrame() {}

//##############################
//			PlayerFrame
//##############################

PlayerFrame::PlayerFrame() {}

PlayerFrame::PlayerFrame(TransformComponent& in_transform_c,
                         PlayerComponent& in_player_c_,
                         PhysicsComponent& in_phys_c) {
  //
  this->position_ = in_transform_c.position;
  this->rotation_ = in_transform_c.rotation;
  this->scale_ = in_transform_c.scale;

  pc_sprinting_ = in_player_c_.sprinting;
  pc_running_ = in_player_c_.running;
  pc_jumping_ = in_player_c_.jumping;
  pc_vel_dir_ = in_player_c_.vel_dir;
  pc_look_dir_ = in_player_c_.look_dir;
  pc_move_dir_ = in_player_c_.move_dir;

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

  ret_ptr->pc_sprinting_ = this->pc_sprinting_;
  ret_ptr->pc_jumping_ = this->pc_jumping_;
  ret_ptr->pc_running_ = this->pc_running_;
  ret_ptr->pc_vel_dir_ = this->pc_vel_dir_;
  ret_ptr->pc_look_dir_ = this->pc_look_dir_;
  ret_ptr->pc_move_dir_ = this->pc_move_dir_;

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
  if (this->pc_sprinting_ != future_pf.pc_sprinting_) {
    return true;
  }
  if (this->pc_running_ != future_pf.pc_running_) {
    return true;
  }
  if (this->pc_jumping_ != future_pf.pc_jumping_) {
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
      ret_frame->pc_sprinting_ = this->pc_sprinting_;
      ret_frame->pc_jumping_ = this->pc_jumping_;
      ret_frame->pc_running_ = this->pc_running_;
      ret_frame->pc_vel_dir_ = this->pc_vel_dir_;
      ret_frame->pc_look_dir_ = this->pc_look_dir_;
      ret_frame->pc_move_dir_ = this->pc_move_dir_;
    } else {
      ret_frame->velocity_ = point_b.velocity_;
      ret_frame->pc_sprinting_ = point_b.pc_sprinting_;
      ret_frame->pc_jumping_ = point_b.pc_jumping_;
      ret_frame->pc_running_ = point_b.pc_running_;
      ret_frame->pc_vel_dir_ = point_b.pc_vel_dir_;
      ret_frame->pc_look_dir_ = point_b.pc_look_dir_;
      ret_frame->pc_move_dir_ = point_b.pc_move_dir_;
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

  in_player_c_.jumping = pc_jumping_;
  in_player_c_.sprinting = pc_sprinting_;
  in_player_c_.running = pc_running_;
  in_player_c_.vel_dir = pc_vel_dir_;
  in_player_c_.look_dir = pc_look_dir_;
  in_player_c_.move_dir = pc_move_dir_;
  in_phys_c.velocity = velocity_;
}

//##############################
//			BallFrame
//##############################

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

//##############################
//			PickUpFrame
//##############################

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
        this->position_ + (point_b.position_ - position_) * percentage_a;

    // ROTATION
    ret_frame->rotation_ =
        glm::slerp(rotation_, point_b.rotation_, percentage_a);

    // SCALE
    // ret_frame->scale_ = this->scale_;

    return ret_frame;

  } catch (std::bad_cast exp) {
    GlobalSettings::Access()->WriteError(__FILE__, __FUNCTION__, "Bad cast");
    return nullptr;
  }

  return nullptr;
}

bool PickUpFrame::ThresholdCheck(DataFrame& in_future_df) {
  //// Cast to PickupFrame
  //PickUpFrame& future_pf = dynamic_cast<PickUpFrame&>(in_future_df);

  //float threshold = 0.0f;

  //// POSITION
  //float pos_diff = glm::distance(position_, future_pf.position_);
  //threshold =
  //    GlobalSettings::Access()->ValueOf("REPLAY_THRESHOLD_PICKUP_POSITION");
  //if (pos_diff > threshold) {
  //  // If we have moved over the threshold value away
  //  return true;
  //}

  //// ROTATION
  //float rot_diff = glm::dot(rotation_, future_pf.rotation_);
  //threshold =
  //    GlobalSettings::Access()->ValueOf("REPLAY_THRESHOLD_PICKUP_ROTATION");
  //if (abs(rot_diff - 1.0f) > threshold) {
  //  // If we have rotated more than the theshhold value allows
  //  return true;
  //}

  return false;
}

void PickUpFrame::WriteBack(TransformComponent& in_transform_c) {
  in_transform_c.position = this->position_;
  in_transform_c.rotation = this->rotation_;
  // in_transform_c.scale = this->scale_;
  in_transform_c.scale = glm::vec3(0.4f);
}

//##############################
//			WallFrame
//##############################

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

bool ShotFrame::ThresholdCheck(DataFrame& in_future_df) {
  // Cast to TeleportShotFrame
  ShotFrame& future_sf = dynamic_cast<ShotFrame&>(in_future_df);

  float threshold = 0.0f;

  // POSITION
  float pos_diff = glm::distance(this->position_, future_sf.position_);
  threshold =
      GlobalSettings::Access()->ValueOf("REPLAY_THRESHOLD_SHOT_POSITION");
  if (pos_diff > threshold) {
    // If we have moved over the threshold value away
    return true;
  }

  return false;
}

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
  // Cast to TeleportShotFrame
  TeleportShotFrame& future_tsf =
      dynamic_cast<TeleportShotFrame&>(in_future_df);

  float threshold = 0.0f;

  // POSITION
  float pos_diff = glm::distance(this->position_, future_tsf.position_);
  threshold = GlobalSettings::Access()->ValueOf(
      "REPLAY_THRESHOLD_TELEPORT_SHOT_POSITION");
  if (pos_diff > threshold) {
    // If we have moved over the threshold value away
    return true;
  }

  return false;
}

DataFrame* TeleportShotFrame::InterpolateForward(
    unsigned int in_dist_to_target, unsigned int in_dist_to_point_b,
    DataFrame& in_point_b) {
  // Cast the DataFrame to TeleportShotFrame
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

//##############################
//			MissileFrame
//##############################

MissileFrame::MissileFrame() {}

MissileFrame::MissileFrame(TransformComponent& in_transform_c) {
  this->position_ = in_transform_c.position;
  this->rotation_ = in_transform_c.rotation;
}

MissileFrame::~MissileFrame() {}

MissileFrame* MissileFrame::Clone() {
  MissileFrame* ret_ptr = new MissileFrame();

  ret_ptr->position_ = this->position_;
  ret_ptr->rotation_ = this->rotation_;

  return ret_ptr;
}

bool MissileFrame::ThresholdCheck(DataFrame& in_future_df) {
  // Cast to BallFrame
  MissileFrame& future_mf = dynamic_cast<MissileFrame&>(in_future_df);

  float threshold = 0.0f;

  // POSITION
  float pos_diff = glm::distance(this->position_, future_mf.position_);
  threshold =
      GlobalSettings::Access()->ValueOf("REPLAY_THRESHOLD_MISSILE_POSITION");
  if (pos_diff > threshold) {
    // If we have moved over the threshold value away
    return true;
  }

  // ROTATION
  float rot_diff = glm::dot(this->rotation_, future_mf.rotation_);
  threshold =
      GlobalSettings::Access()->ValueOf("REPLAY_THRESHOLD_MISSILE_ROTATION");
  if (abs(rot_diff - 1.0f) > threshold) {
    // If we have rotated more than the theshhold value allows
    return true;
  }

  return false;
}

DataFrame* MissileFrame::InterpolateForward(unsigned int in_dist_to_target,
                                            unsigned int in_dist_to_point_b,
                                            DataFrame& in_point_b) {
  // Cast the DataFrame to MissileFrame
  try {
    MissileFrame& point_b = dynamic_cast<MissileFrame&>(in_point_b);
    // Skips forward if std::bad_cast

    // INTERPOLATED FRAME
    MissileFrame* ret_frame = new MissileFrame();

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

    return ret_frame;

  } catch (std::bad_cast exp) {
    GlobalSettings::Access()->WriteError(__FILE__, __FUNCTION__, "Bad cast");
    return nullptr;
  }
}

void MissileFrame::WriteBack(TransformComponent& in_transform_c) {
  in_transform_c.position = this->position_;
  in_transform_c.rotation = this->rotation_;
  in_transform_c.scale = glm::vec3(0.5f);
}

//##############################
//			ForcePushFrame
//##############################

ForcePushFrame::ForcePushFrame() {
  position_ = glm::vec3(0.f);
  rotation_ = glm::quat();
}

ForcePushFrame::ForcePushFrame(TransformComponent& trans_c) {
  position_ = trans_c.position;
  rotation_ = trans_c.rotation;
}

ForcePushFrame::~ForcePushFrame() {}

DataFrame* ForcePushFrame::Clone() {
  ForcePushFrame* force_push_return = new ForcePushFrame();

  force_push_return->position_ = position_;
  force_push_return->rotation_ = rotation_;

  return force_push_return;
}

DataFrame* ForcePushFrame::InterpolateForward(unsigned int in_dist_to_target,
                                              unsigned int in_dist_to_point_b,
                                              DataFrame& in_point_b) {
  // Cast the DataFrame to ForcePushFrame
  try {
    ForcePushFrame& point_b = dynamic_cast<ForcePushFrame&>(in_point_b);
    // Skips forward if std::bad_cast

    // INTERPOLATED FRAME
    ForcePushFrame* ret_frame = new ForcePushFrame();

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
    ret_frame->rotation_ =
        glm::slerp(rotation_, point_b.rotation_, percentage_a);

    return ret_frame;

  } catch (std::bad_cast exp) {
    GlobalSettings::Access()->WriteError(__FILE__, __FUNCTION__, "Bad cast");
    return nullptr;
  }
}

bool ForcePushFrame::ThresholdCheck(DataFrame& in_future_df) {
  // Cast to ForcePushFrame
  ForcePushFrame& future_pf = dynamic_cast<ForcePushFrame&>(in_future_df);

  float threshold = 0.0f;

  // POSITION
  float pos_diff = glm::distance(position_, future_pf.position_);
  threshold =
      GlobalSettings::Access()->ValueOf("REPLAY_THRESHOLD_FORCE_PUSH_POSITION");
  if (pos_diff > threshold) {
    // If we have moved over the threshold value away
    return true;
  }

  return false;
}

void ForcePushFrame::WriteBack(TransformComponent& trans_c) {
  trans_c.position = position_;
  trans_c.rotation = rotation_;
  trans_c.scale = glm::vec3(0.5f);
}

//##############################
//			MineFrame
//##############################

MineFrame::MineFrame() { position_ = glm::vec3(0.f); }

MineFrame::MineFrame(TransformComponent& trans_c) {
  position_ = trans_c.position;
}

MineFrame::~MineFrame() {}

DataFrame* MineFrame::Clone() {
  MineFrame* mine_return = new MineFrame();

  mine_return->position_ = position_;

  return mine_return;
}

DataFrame* MineFrame::InterpolateForward(unsigned int in_dist_to_target,
                                         unsigned int in_dist_to_point_b,
                                         DataFrame& in_point_b) {
  // INTERPOLATED FRAME
  MineFrame* ret_frame = new MineFrame();

  // "INTERPOLATION" :D

  // POSITION
  ret_frame->position_ = this->position_;

  return ret_frame;
}

bool MineFrame::ThresholdCheck(DataFrame& in_future_df) { return false; }

void MineFrame::WriteBack(TransformComponent& trans_c) {
  trans_c.position = position_;
  trans_c.scale = glm::vec3(1.0f);
}

BlackholeFrame::BlackholeFrame() {
  position_ = glm::vec3(0.f);
  rotation_ = glm::quat();
}

BlackholeFrame::BlackholeFrame(TransformComponent& trans_c) {
  position_ = trans_c.position;
  rotation_ = trans_c.rotation;
}

BlackholeFrame::~BlackholeFrame() {}

DataFrame* BlackholeFrame::Clone() {
  BlackholeFrame* blackhole_return = new BlackholeFrame();

  blackhole_return->position_ = position_;
  blackhole_return->rotation_ = rotation_;

  return blackhole_return;
}

DataFrame* BlackholeFrame::InterpolateForward(unsigned int in_dist_to_target,
                                              unsigned int in_dist_to_point_b,
                                              DataFrame& in_point_b) {
  // Cast the BlackholeFrame to ForcePushFrame
  try {
    BlackholeFrame& point_b = dynamic_cast<BlackholeFrame&>(in_point_b);
    // Skips forward if std::bad_cast

    // INTERPOLATED FRAME
    BlackholeFrame* ret_frame = new BlackholeFrame();

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
    ret_frame->rotation_ =
        glm::slerp(rotation_, point_b.rotation_, percentage_a);

    return ret_frame;

  } catch (std::bad_cast exp) {
    GlobalSettings::Access()->WriteError(__FILE__, __FUNCTION__, "Bad cast");
    return nullptr;
  }
}

bool BlackholeFrame::ThresholdCheck(DataFrame& in_future_df) {  // Cast to ForcePushFrame
  BlackholeFrame& future_pf = dynamic_cast<BlackholeFrame&>(in_future_df);

  float threshold = 0.0f;

  // POSITION
  float pos_diff = glm::distance(position_, future_pf.position_);
  threshold =
      GlobalSettings::Access()->ValueOf("REPLAY_THRESHOLD_BLACKHOLE_POSITION");
  if (pos_diff > threshold) {
    // If we have moved over the threshold value away
    return true;
  }

  return false;
}

void BlackholeFrame::WriteBack(TransformComponent& trans_c) {
  trans_c.position = position_;
  trans_c.rotation = rotation_;
  trans_c.scale = glm::vec3(0.3f);
}
