#include "geometric_replay.hpp"

#include <map>

#include <ecs/components/ball_component.hpp>
#include <ecs/components/player_component.hpp>
#include <ecs/components/model_component.hpp>
#include <shared/transform_component.hpp>
#include <util/global_settings.hpp>
#include <glob/graphics.hpp>
#include <util/asset_paths.hpp>
    // Private---------------------------------------------------------------------

ReplayObjectType GeometricReplay::IdentifyEntity(entt::entity& in_entity,
                                                 entt::registry& in_registry) {
  if (in_registry.has<PlayerComponent>(in_entity)) {
    return ReplayObjectType::REPLAY_PLAYER;
  } else if (in_registry.has<BallComponent>(in_entity)) {
    return ReplayObjectType::REPLAY_BALL;
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

  switch (object_type) {
    case REPLAY_PLAYER:
      TransformComponent& transform_c =
          in_registry.get<TransformComponent>(in_entity);
      PlayerComponent& player_c = in_registry.get<PlayerComponent>(in_entity);

      ret_ptr = new PlayerFrame(transform_c, player_c);
      break;
    case REPLAY_BALL:
      TransformComponent& transform_c =
          in_registry.get<TransformComponent>(in_entity);

      ret_ptr = new BallFrame(transform_c);
      break;
    default:
      GlobalSettings::Access()->WriteError(__FILE__, __FUNCTION__,
                                           "Unidentified entity");
      break;
  }

  return ret_ptr;
}

void GeometricReplay::InterpolateEntityData(unsigned int in_channel_index,
                                            entt::entity& in_entity,
                                            entt::registry& in_registry) {
  // Check the current reading frame and determine the indices
  // for interpolation points 'a' & 'b' in the FrameChannel
  unsigned int& c = this->current_frame_number_read_;
  unsigned int& a = this->channels_.at(in_channel_index).index_a;
  unsigned int& b = this->channels_.at(in_channel_index).index_b;

  // If the current frame is at or has surpassed the frame at point 'b'
  if (c >= this->channels_.at(in_channel_index).entries.at(b).frame_number) {
    // Move 'a' forward to 'b'
    // Note that we move 'a' to 'b', and do not increment it
    // This is to prevent 'a' going out of scope
    a = b;
    // Move 'b' forward to the next index if it won't go
    // over the channel's last entry
    if (b + 1 != this->channels_.at(in_channel_index).entries.size()) {
      b++;
    }
  }

  // Get the DataFrame at 'a' and interpolate it forwards
  // towards 'b' to find the DataFrame representing the current frame
  unsigned int dist_to_target =
      this->current_frame_number_read_ -
      this->channels_.at(in_channel_index).entries.at(a).frame_number;
  unsigned int dist_to_b =
      this->channels_.at(in_channel_index).entries.at(b).frame_number -
      this->channels_.at(in_channel_index).entries.at(a).frame_number;

  // If the distance to the target is greater than or equal to the
  // distance to 'b' we have passed the last entry in the channel
  if (dist_to_target >= dist_to_b) {
    // We set the distance to the target to be the distance
    // to 'b', saying that all frames past the last entry
    // has the value of the last entry
    dist_to_target = dist_to_b;
  }

  DataFrame* df_a_ptr =
      this->channels_.at(in_channel_index).entries.at(a).data_ptr;
  DataFrame* df_b_ptr =
      this->channels_.at(in_channel_index).entries.at(b).data_ptr;

  DataFrame* df_c_ptr =
      df_a_ptr->InterpolateForward(dist_to_target, dist_to_b, (*df_b_ptr));
  // NTS:	InterpolateForward allocates new memory space
  //		Remember to clean up!

  // Load the data from the created frame into the
  // components of the entity
  ReplayObjectType object_type =
      this->channels_.at(in_channel_index).object_type;
  this->DepolymorphFromDataframe(df_c_ptr, object_type, in_entity, in_registry);

  // De-allocate and clean up after InterpolateForward()
  delete df_c_ptr;
}

void GeometricReplay::DepolymorphFromDataframe(DataFrame* in_df_ptr,
                                               ReplayObjectType in_type,
                                               entt::entity& in_entity,
                                               entt::registry& in_registry) {
  switch (in_type) {
    case REPLAY_PLAYER:
      // Cast
      PlayerFrame* pf_c_ptr = dynamic_cast<PlayerFrame*>(in_df_ptr);
      // Get
      TransformComponent& transform_c =
          in_registry.get<TransformComponent>(in_entity);
      PlayerComponent& player_c = in_registry.get<PlayerComponent>(in_entity);
      // Transfer
      pf_c_ptr->WriteBack(transform_c, player_c);
      //
      // WIP: Handle model component
      //
      break;
    case REPLAY_BALL:
      // Cast
      BallFrame* bf_c_ptr = dynamic_cast<BallFrame*>(in_df_ptr);
      // Get
      TransformComponent& transform_c =
          in_registry.get<TransformComponent>(in_entity);
      // Transfer
      bf_c_ptr->WriteBack(transform_c);
      //
      // WIP: Handle model component
      //
      break;
    default:
      GlobalSettings::Access()->WriteError(__FILE__, __FUNCTION__,
                                           "Unknown type identifier");
      break;
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

  // Switch-case over object types
  // In each:
  //	- Read data and assign components for that entity type
  switch (object_type) {
    case REPLAY_PLAYER:
      PlayerFrame* pf_ptr = dynamic_cast<PlayerFrame*>(df_ptr);
      in_registry.assign<IDComponent>(
          entity, this->channels_.at(in_channel_index).object_id);

	  //
      TransformComponent& transform_c =
          in_registry.assign<TransformComponent>(entity);
      PlayerComponent& player_c=
          in_registry.assign<PlayerComponent>(entity);
      pf_ptr->WriteBack(transform_c, player_c);

      // Create and add ModelHandle
      glob::ModelHandle mh_mech = glob::GetModel(kModelPathMech);
      ModelComponent& model_c = in_registry.assign<ModelComponent>(entity);
      model_c.handles.push_back(mh_mech);

      break;
    case REPLAY_BALL:
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

      break;
    default:
      GlobalSettings::Access()->WriteError(__FILE__, __FUNCTION__,
                                           "Unknown type identifier");
      break;
  }
}

// Protected-------------------------------------------------------------------

GeometricReplay::GeometricReplay() {
  // Empty constructor
}

// Public----------------------------------------------------------------------

GeometricReplay::GeometricReplay(unsigned int in_replay_length_sec,
                                 unsigned int in_frames_per_sec) {
  this->threshhold_age_ = in_replay_length_sec * in_frames_per_sec;
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
    for (unsigned int i = 0; i < this->channels_.size && id_unfound; i++) {
      if (id_c.id == this->channels_.at(i).object_id) {
        id_unfound = false;
        // If it does we check if a new interpolation point should be added
        // dependent on the object's type
        DataFrame* temp_df = nullptr;
        ReplayObjectType temp_object_type =
            this->IdentifyEntity(entity, in_registry);

        switch (temp_object_type) {
          case REPLAY_PLAYER:
            TransformComponent& transform_c =
                in_registry.get<TransformComponent>(entity);
            PlayerComponent& player_c =
                in_registry.get<PlayerComponent>(entity);
            temp_df = new PlayerFrame(transform_c, player_c);
            break;
          case REPLAY_BALL:
            TransformComponent& transform_c =
                in_registry.get<TransformComponent>(entity);
            temp_df = new BallFrame(transform_c);
            break;
          default:
            GlobalSettings::Access()->WriteError(__FILE__, __FUNCTION__,
                                                 "Unknown FrameType");
            break;
        }

        // Compare last entry to what would be the current frame's
        if ((temp_df != nullptr) &&
            this->channels_.at(i).entries.back().data_ptr->ThresholdCheck(
                *temp_df)) {
          // IF true, save DataFrame
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
  //	3. If so, remove first entry in channel OR remove entire
  //	channel if the second entry marks an ending entry*
  //
  // *When an object disappears from the game world (and thus the replay)
  // register that as an "ending entry".
  for (unsigned int i = 0; i < this->channels_.size(); i++) {
    if (this->channels_.at(i).entries.size() > 1) {  //(1.)
      unsigned int age = this->current_frame_number_write_ -
                         this->channels_.at(i).entries.at(1).frame_number;
      if (age > this->threshhold_age_) {                         //(2.)
        if (this->channels_.at(i).entries.at(1).ending_entry) {  //(3.)
          this->channels_.erase(this->channels_.begin() + i,
                                this->channels_.begin + i);
          i--;
        } else {
          this->channels_.at(i).entries.erase(
              this->channels_.at(i).entries.begin(),
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

  return true;
}

void GeometricReplay::SetWriteFrame(unsigned int in_frame_number) {
  this->current_frame_number_write_ = in_frame_number;
}

void GeometricReplay::SetReadFrame(unsigned int in_frame_number) {
  this->current_frame_number_read_ = in_frame_number;
}
