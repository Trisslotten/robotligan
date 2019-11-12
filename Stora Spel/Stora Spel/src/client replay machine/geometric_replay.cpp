#include "geometric_replay.hpp"

#include <map>

#include <ecs/components.hpp>
#include <glob/graphics.hpp>
#include <shared/pick_up_component.hpp>
#include <shared/transform_component.hpp>
#include <util/asset_paths.hpp>
#include <util/global_settings.hpp>
// Private---------------------------------------------------------------------

ReplayObjectType GeometricReplay::IdentifyEntity(entt::entity& in_entity,
                                                 entt::registry& in_registry) {
  //
  // NTS: DATAFRAME IF CASE
  // - Identify given entity type
  // - Return enum type (enum defined in geometric_replay.hpp)
  //

  if (in_registry.has<PlayerComponent>(in_entity)) {
    return ReplayObjectType::REPLAY_PLAYER;
  } else if (in_registry.has<BallComponent>(in_entity)) {
    return ReplayObjectType::REPLAY_BALL;
  } else if (in_registry.has<PickUpComponent>(in_entity)) {
    return ReplayObjectType::REPLAY_PICKUP;
  } else if (in_registry.has<ProjectileComponent>(in_entity)) {
    ProjectileComponent& proj_c =
        in_registry.get<ProjectileComponent>(in_entity);
    if (proj_c.projectile_id == ProjectileID::CANNON_BALL) {
      return ReplayObjectType::REPLAY_SHOT;
    } else if (proj_c.projectile_id == ProjectileID::TELEPORT_PROJECTILE) {
      return ReplayObjectType::REPLAY_TELEPORT_SHOT;
    }
  }

  // If entity couldn't be identified, return number of types
  return ReplayObjectType::NUM_OF_REPLAY_OBJECT_TYPES;
}

void GeometricReplay::FillChannelEntry(ChannelEntry& in_ce,
                                       entt::entity& in_entity,
                                       entt::registry& in_registry) {
  // Channel entry frame
  in_ce.frame_number = this->current_frame_number_write_;

  // Channel entry data
  DataFrame* df_ptr = this->PolymorphIntoDataFrame(in_entity, in_registry);

  // Then save
  in_ce.data_ptr = df_ptr;

  // Channel entry is ending entry?
  // Here: No
  in_ce.ending_entry = false;
}

DataFrame* GeometricReplay::PolymorphIntoDataFrame(
    entt::entity& in_entity, entt::registry& in_registry) {
  DataFrame* ret_ptr = nullptr;
  // Start by identifying what kind of entity we
  // are trying to save

  ReplayObjectType object_type = this->IdentifyEntity(in_entity, in_registry);

  //
  // NTS: DATAFRAME IF CASE
  // - Fetch relevant components
  // - Pass them to a new DataFrame for the object type
  //

  if (object_type == REPLAY_PLAYER) {
    TransformComponent& transform_c =
        in_registry.get<TransformComponent>(in_entity);
    PlayerComponent& player_c = in_registry.get<PlayerComponent>(in_entity);

    ret_ptr = new PlayerFrame(transform_c, player_c);
  } else if (object_type == REPLAY_BALL) {
    TransformComponent& transform_c =
        in_registry.get<TransformComponent>(in_entity);

    ret_ptr = new BallFrame(transform_c);
  } else if (object_type == REPLAY_PICKUP) {
    // TBA
  } else if (object_type == REPLAY_SHOT) {
    TransformComponent& transform_c =
        in_registry.get<TransformComponent>(in_entity);

    ret_ptr = new ShotFrame(transform_c);
  } else if (object_type == REPLAY_TELEPORT_SHOT) {
    TransformComponent& transform_c =
        in_registry.get<TransformComponent>(in_entity);

    ret_ptr = new TeleportShotFrame(transform_c);
  } else {
    GlobalSettings::Access()->WriteError(__FILE__, __FUNCTION__,
                                         "Unidentified entity");
  }

  return ret_ptr;
}

DataFrame* GeometricReplay::InterpolateDataFrame(
    unsigned int in_channel_index, unsigned int in_entry_index_a,
    unsigned int in_entry_index_b, unsigned int in_target_frame_num) {
  // Get the DataFrame at 'a' and interpolate it forwards
  // towards 'b' to find the DataFrame representing the current frame

  // Prevent 'b' index from being out of scope
  // This can occur if there is only one entry in
  // channel
  if (in_entry_index_b <= this->channels_.at(in_channel_index).entries.size()) {
    in_entry_index_b = in_entry_index_a;
  }

  // Calculate distances in 'frames'
  unsigned int dist_to_target =
      in_target_frame_num - this->channels_.at(in_channel_index)
                                .entries.at(in_entry_index_a)
                                .frame_number;
  unsigned int dist_to_b = this->channels_.at(in_channel_index)
                               .entries.at(in_entry_index_b)
                               .frame_number -
                           this->channels_.at(in_channel_index)
                               .entries.at(in_entry_index_a)
                               .frame_number;

  // If the distance to the target is greater than or equal to the
  // distance to 'b' we have passed the last entry in the channel
  if (dist_to_target >= dist_to_b) {
    // We set the distance to the target to be the distance
    // to 'b', saying that all frames past the last entry
    // has the value of the last entry
    dist_to_target = dist_to_b;
  }

  DataFrame* df_a_ptr = this->channels_.at(in_channel_index)
                            .entries.at(in_entry_index_a)
                            .data_ptr;
  DataFrame* df_b_ptr = this->channels_.at(in_channel_index)
                            .entries.at(in_entry_index_b)
                            .data_ptr;

  DataFrame* df_c_ptr =
      df_a_ptr->InterpolateForward(dist_to_target, dist_to_b, (*df_b_ptr));
  // NTS:	InterpolateForward allocates new memory space
  //		Remember to clean up!

  return df_c_ptr;
}

void GeometricReplay::InterpolateEntityData(unsigned int in_channel_index,
                                            entt::entity& in_entity,
                                            entt::registry& in_registry) {
  // Check the current reading frame,'c', and determine the indices
  // for interpolation points 'a' & 'b' in the FrameChannel
  unsigned int& c_frame = this->current_frame_number_read_;
  unsigned int& a_index = this->channels_.at(in_channel_index).index_a;
  unsigned int& b_index = this->channels_.at(in_channel_index).index_b;

  // If the current frame is at or has surpassed the frame at point 'b'
  if (c_frame >=
      this->channels_.at(in_channel_index).entries.at(b_index).frame_number) {
    // Move 'a' forward to 'b'
    // Note that we move 'a' to 'b', and do not increment it
    // This is to prevent 'a' going out of scope
    a_index = b_index;
    // Move 'b' forward to the next index if it won't go
    // over the channel's last entry
    if (b_index != (this->channels_.at(in_channel_index).entries.size() - 1)) {
      b_index++;
    }
  }

  // Interpolate a new frame (allocates memory)
  DataFrame* interpolated_frame_ptr =
      this->InterpolateDataFrame(in_channel_index, a_index, b_index, c_frame);

  // Load the data from the created frame into the
  // components of the entity
  ReplayObjectType object_type =
      this->channels_.at(in_channel_index).object_type;
  this->DepolymorphFromDataframe(interpolated_frame_ptr, object_type, in_entity,
                                 in_registry);

  // De-allocate to clean up after InterpolateForward()
  delete interpolated_frame_ptr;
}

void GeometricReplay::DepolymorphFromDataframe(DataFrame* in_df_ptr,
                                               ReplayObjectType in_type,
                                               entt::entity& in_entity,
                                               entt::registry& in_registry) {
  //
  // NTS: DATAFRAME IF CASE
  // - Cast DataFrame to correct type
  // - Fetch relevant components
  // - Pass them to the casted DataFrame's WriteBack function
  //

  if (in_type == REPLAY_PLAYER) {
    // Cast
    PlayerFrame* pf_c_ptr = dynamic_cast<PlayerFrame*>(in_df_ptr);
    // Get
    TransformComponent& transform_c =
        in_registry.get<TransformComponent>(in_entity);
    PlayerComponent& player_c = in_registry.get<PlayerComponent>(in_entity);
    // Transfer
    pf_c_ptr->WriteBack(transform_c, player_c);
  } else if (in_type == REPLAY_BALL) {
    // Cast
    BallFrame* bf_c_ptr = dynamic_cast<BallFrame*>(in_df_ptr);
    // Get
    TransformComponent& transform_c =
        in_registry.get<TransformComponent>(in_entity);
    // Transfer
    bf_c_ptr->WriteBack(transform_c);
  } else if (in_type == REPLAY_PICKUP) {
    // TBA
  } else if (in_type == REPLAY_SHOT) {
    // Cast
    ShotFrame* sf_c_ptr = dynamic_cast<ShotFrame*>(in_df_ptr);
    // Get
    TransformComponent& transform_c =
        in_registry.get<TransformComponent>(in_entity);
    // Transfer
    sf_c_ptr->WriteBack(transform_c);
  } else if (in_type == REPLAY_TELEPORT_SHOT) {
    // Cast
    TeleportShotFrame* tsf_c_ptr = dynamic_cast<TeleportShotFrame*>(in_df_ptr);
    // Get
    TransformComponent& transform_c =
        in_registry.get<TransformComponent>(in_entity);
    // Transfer
    tsf_c_ptr->WriteBack(transform_c);
  } else {
    GlobalSettings::Access()->WriteError(__FILE__, __FUNCTION__,
                                         "Unknown type identifier");
  }
}

void GeometricReplay::CreateEntityFromChannel(unsigned int in_channel_index,
                                              entt::registry& in_registry) {
  // Get the first frame of the channel that is tracking the entity
  DataFrame* df_ptr =
      this->channels_.at(in_channel_index).entries.at(0).data_ptr;

  // Fetch FrameType
  ReplayObjectType object_type =
      this->channels_.at(in_channel_index).object_type;

  // Create base entity
  entt::entity entity = in_registry.create();

  //
  // NTS: DATAFRAME IF CASE
  // - Cast DataFrame to correct type
  // - Assign the relevant components to entity
  // - Assign a model component to the entity
  // - Add the relevant ModelHandle:s to entity
  //
  if (object_type == REPLAY_PLAYER) {
    PlayerFrame* pf_ptr = dynamic_cast<PlayerFrame*>(df_ptr);
    in_registry.assign<IDComponent>(
        entity, this->channels_.at(in_channel_index).object_id);

    //
    TransformComponent& transform_c =
        in_registry.assign<TransformComponent>(entity);
    PlayerComponent& player_c = in_registry.assign<PlayerComponent>(entity);
    pf_ptr->WriteBack(transform_c, player_c);

    // Create and add ModelHandle
    glob::ModelHandle mh_mech = glob::GetModel(kModelPathMech);
    ModelComponent& model_c = in_registry.assign<ModelComponent>(entity);
    model_c.handles.push_back(mh_mech);
  } else if (object_type == REPLAY_BALL) {
    BallFrame* bf_ptr = dynamic_cast<BallFrame*>(df_ptr);
    in_registry.assign<IDComponent>(
        entity, this->channels_.at(in_channel_index).object_id);

    //
    TransformComponent& transform_c =
        in_registry.assign<TransformComponent>(entity);
    bf_ptr->WriteBack(transform_c);

    // Create and add ModelHandle
    glob::ModelHandle mh_ball_proj = glob::GetModel(kModelPathBallProjectors);
    glob::ModelHandle mh_ball_sphe = glob::GetModel(kModelPathBallSphere);
    ModelComponent& model_c = in_registry.assign<ModelComponent>(entity);
    model_c.handles.push_back(mh_ball_proj);
    model_c.handles.push_back(mh_ball_sphe);
  } else if (object_type == REPLAY_PICKUP) {
    // TBA
  } else if (object_type == REPLAY_SHOT) {
    ShotFrame* bf_ptr = dynamic_cast<ShotFrame*>(df_ptr);
    in_registry.assign<IDComponent>(
        entity, this->channels_.at(in_channel_index).object_id);

    //
    TransformComponent& transform_c =
        in_registry.assign<TransformComponent>(entity);
    bf_ptr->WriteBack(transform_c);

    // Create and add ModelHandle
    glob::ModelHandle mh_shot = glob::GetModel(kModelPathRocket);
    ModelComponent& model_c = in_registry.assign<ModelComponent>(entity);
    model_c.handles.push_back(mh_shot);
  } else if (object_type == REPLAY_TELEPORT_SHOT) {
    TeleportShotFrame* tsf_ptr = dynamic_cast<TeleportShotFrame*>(df_ptr);
    in_registry.assign<IDComponent>(
        entity, this->channels_.at(in_channel_index).object_id);

    //
    TransformComponent& transform_c =
        in_registry.assign<TransformComponent>(entity);
    tsf_ptr->WriteBack(transform_c);

    // Create and add ModelHandle
    glob::ModelHandle mh_shot = glob::GetModel(kModelPathPickup);
    ModelComponent& model_c = in_registry.assign<ModelComponent>(entity);
    model_c.handles.push_back(mh_shot);

    // Fix with trail here
  } else {
    GlobalSettings::Access()->WriteError(__FILE__, __FUNCTION__,
                                         "Unknown type identifier");
  }
}

// Protected-------------------------------------------------------------------

GeometricReplay::GeometricReplay() {
  // "Empty" constructor
  this->threshhold_age_ = 0;
  this->current_frame_number_write_ = 0;
  this->current_frame_number_read_ = 0;
}

// Public----------------------------------------------------------------------

GeometricReplay::GeometricReplay(unsigned int in_replay_length_sec,
                                 unsigned int in_frames_per_sec) {
  this->threshhold_age_ = in_replay_length_sec * in_frames_per_sec;

  this->current_frame_number_write_ = 0;
  this->current_frame_number_read_ = 0;
}

GeometricReplay::~GeometricReplay() {}

GeometricReplay* GeometricReplay::Clone() {
  GeometricReplay* clone = new GeometricReplay();

  clone->channels_ = std::vector<FrameChannel>(this->channels_);
  // NTS: According to the internet the above should produce a deep copy
  clone->threshhold_age_ = this->threshhold_age_;
  clone->current_frame_number_write_ = this->current_frame_number_write_;
  clone->current_frame_number_read_ = this->current_frame_number_read_;

  return clone;
}

bool GeometricReplay::SaveFrame(entt::registry& in_registry) {
  // Loop over all entries with an id component
  entt::basic_view view = in_registry.view<IDComponent>();
  for (entt::entity entity : view) {
    IDComponent& id_c = in_registry.get<IDComponent>(entity);

    // Check if entity with that id has a channel
    bool id_unfound = true;
    for (unsigned int i = 0; i < this->channels_.size() && id_unfound; i++) {
      if (id_c.id == this->channels_.at(i).object_id) {
        id_unfound = false;
        // If it does we check if a new interpolation point should be added
        // dependent on the object's type
        DataFrame* temp_df = nullptr;
        temp_df = this->PolymorphIntoDataFrame(entity, in_registry);

        // Compare last entry to what would be the current frame's
        if ((temp_df != nullptr) &&
            this->channels_.at(i).entries.back().data_ptr->ThresholdCheck(
                *temp_df)) {
          ChannelEntry temp_ce;
          temp_ce.frame_number = this->current_frame_number_write_;
          temp_ce.data_ptr = temp_df;
          temp_ce.ending_entry = false;
          this->channels_.at(i).entries.push_back(temp_ce);
        }
      }
    }

    // If after looping through an object still hasn't been found
    // it should be added to its own channel
    if (id_unfound) {
      // Entry
      ChannelEntry temp_ce;
      this->FillChannelEntry(temp_ce, entity, in_registry);

      // Channel
      FrameChannel temp_fc;
      temp_fc.object_type = this->IdentifyEntity(entity, in_registry);
      temp_fc.object_id = id_c.id;
      temp_fc.entries.push_back(temp_ce);

      this->channels_.push_back(temp_fc);
    }
  }

  // Check the first entry in each channel
  // Its age can be calculated by checking the current number of
  // frames passed and comparing it to the frame it was saved on
  // We do not want to record the entire match (due to memory consumption)
  // so therefore:
  //	1. Check if there are more than one entry in channel
  //	2. If so, check if second entry is over age threshold
  //	3. If so, remove first entry in channel (3.2) OR remove entire
  //	channel if the second entry marks an ending entry* (3.1)
  //
  // *When an object disappears from the game world (and thus the replay)
  // register that as an "ending entry".
  for (unsigned int i = 0; i < this->channels_.size(); i++) {
    if (this->channels_.at(i).entries.size() > 1) {  //(1.)
      unsigned int age = this->current_frame_number_write_ -
                         this->channels_.at(i).entries.at(1).frame_number;
      if (age > this->threshhold_age_) {                         //(2.)
        if (this->channels_.at(i).entries.at(1).ending_entry) {  //(3.1)
          this->channels_.erase(this->channels_.begin() + i);
          i--;
        } else {  //(3.2)
          this->channels_.at(i).entries.erase(
              this->channels_.at(i).entries.begin());
        }
      }
    }
  }

  // After having saved a frame, increment
  this->current_frame_number_write_++;

  return false;
}

bool GeometricReplay::LoadFrame(entt::registry& in_registry) {
  // Return true if end of the replay has been reached
  // (that is to say: the read has caught up with the write)
  if (this->current_frame_number_read_ >= this->current_frame_number_write_) {
    return true;
  }

  // Get all entities with an IDComponent
  entt::basic_view view = in_registry.view<IDComponent>();

  // Loop over all channels
  for (unsigned int i = 0; i < this->channels_.size(); i++) {
    // Try finding the current channel's ID in the view
    bool id_unfound = true;

    for (entt::entity entity : view) {
      IDComponent& id_c = in_registry.get<IDComponent>(entity);

      if (this->channels_.at(i).object_id == id_c.id) {
        // If the id is found set the entity's
        // component data using the interpolation function
        this->InterpolateEntityData(i, entity, in_registry);

        // Note that we found id and jump out of for-loop
        id_unfound = false;
        break;
      }
    }

    if (id_unfound) {
      // Create the entity that existed in the replay
      // but not in the registry
      this->CreateEntityFromChannel(i, in_registry);
    }
  }

  // Increment read index
  this->current_frame_number_read_++;

  return false;
}

void GeometricReplay::SetReadFrameToStart() {
  // Prevent looping
  if (this->current_frame_number_write_ < this->threshhold_age_) {
    this->current_frame_number_read_ = 0;
    return;
  }

  this->current_frame_number_read_ =
      this->current_frame_number_write_ - this->threshhold_age_;
}

void GeometricReplay::ChannelCatchUp() {
  // Goes through all channels and moves all
  // channels to have their first entry not lie
  // beyond the threshold age

  // Set the number of the first frame(s) as the number
  // the reading tracker starts from
  this->SetReadFrameToStart();

  for (unsigned int i = 0; i < this->channels_.size(); i++) {
    // Check the age of the first entry
    unsigned int age = this->current_frame_number_write_ -
                       this->channels_.at(i).entries.at(0).frame_number;

    // If the age is greater than the threshold
    if (age > this->threshhold_age_) {
      // Create an interpolation that lies right
      // at the the threshold
      // unsigned int threshold_frame = age - this->threshhold_age_;
      unsigned int threshold_frame = this->current_frame_number_read_;

      DataFrame* threshold_frame_ptr =
          this->InterpolateDataFrame(i, 0, 1, threshold_frame);

      // Replace the data in first entry with the just calculated stuff
      this->channels_.at(i).entries.at(0).frame_number = threshold_frame;
      delete this->channels_.at(i).entries.at(0).data_ptr;
      this->channels_.at(i).entries.at(0).data_ptr = threshold_frame_ptr;
    }
  }
}

std::string GeometricReplay::GetGeometricReplayTree() {
  std::string ret_str = "Geometric Replay Tree\n";
  ret_str += "\tThreshold age: " + std::to_string(this->threshhold_age_) + "\n";
  ret_str += "\tChannels:\n\t\t---\n";

  for (unsigned int i = 0; i < this->channels_.size(); i++) {
    ret_str += "\t\tChannel Number: " + std::to_string(i) + "\n";
    ret_str += "\t\tChannel Type: " +
               std::to_string(this->channels_.at(i).object_type) + "\n";
    ret_str += "\t\tChannel Entries: " +
               std::to_string(this->channels_.at(i).entries.size()) + "\n";
    ret_str += "\t\tChannel write ended at frame: " +
               std::to_string(this->current_frame_number_write_) + "\n";
    ret_str += "\t\tChannel read starts at frame: " +
               std::to_string(this->current_frame_number_read_) + "\n";

    unsigned int oea = this->current_frame_number_write_ -
                       this->channels_.at(i).entries.at(0).frame_number;
    unsigned int nea = this->current_frame_number_write_ -
                       this->channels_.at(i).entries.back().frame_number;

    ret_str += "\t\t\tOldest Entry Age: " + std::to_string(oea) + "\n";
    ret_str += "\t\t\tNewest Entry Age: " + std::to_string(nea) + "\n";

    //---
    ret_str += "\t\t\t\t";
    for (unsigned int j = 0; j < this->channels_.at(i).entries.size(); j++) {
      std::string age_str =
          std::to_string(this->current_frame_number_write_ -
                         this->channels_.at(i).entries.at(j).frame_number);
      ret_str += "[" + age_str;
      if (this->channels_.at(i).entries.at(j).ending_entry) {
        ret_str += ":end";
      }
      ret_str += "] ";
    }
    ret_str += "\n";
    //---
    ret_str += "\t\t---\n";
  }

  return ret_str;
}

std::string GeometricReplay::GetStateOfReplay() {
  std::string ret_str = "";

  unsigned int start_frame = 0;
  if (this->current_frame_number_write_ > this->threshhold_age_) {
    start_frame = this->current_frame_number_write_ - this->threshhold_age_;
  }

  ret_str += "\tFrame-stride:\t";
  ret_str += std::to_string(start_frame);
  ret_str += "  [" + std::to_string(this->current_frame_number_read_) + "]  ";
  ret_str += std::to_string(this->current_frame_number_write_);

  return ret_str;
}
