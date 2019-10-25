#include "geometric_replay.hpp"

#include <map>

#include <ecs/components/ball_component.hpp>
#include <ecs/components/player_component.hpp>
#include <shared/transform_component.hpp>

// Private---------------------------------------------------------------------

// Public----------------------------------------------------------------------

void GeometricReplay::FillChannelEntry(ChannelEntry& in_ce,
                                       entt::entity& in_entity,
                                       entt::registry& in_registry) {
  // Channel entry frame
  in_ce.frame_number = this->current_frame_number_;

  // Channel entry data
  DataFrame* df_ptr = nullptr;

  // Start by identifying what kind of entity we
  // are trying to save
  if (in_registry.has<PlayerComponent>(in_entity)) {
    // If there is a player component we know it is
    // a player avatar

    // WIP: Might be needed for animations
    // PlayerComponent& player_c = in_registry.get<PlayerComponent>(in_entity);

    TransformComponent& transform_c =
        in_registry.get<TransformComponent>(in_entity);

    df_ptr = new PlayerFrame(transform_c.position, transform_c.rotation,
                             transform_c.scale);

  } else if (in_registry.has<BallComponent>(in_entity)) {
    // Otherwise, if there is a ball component we
    // know it to be a ball
    TransformComponent& transform_c =
        in_registry.get<TransformComponent>(in_entity);

    df_ptr = new BallFrame(transform_c.position, transform_c.rotation,
                           transform_c.scale);
  }

  // Then save
  in_ce.data_ptr = df_ptr;

  // Channel entry is ending entry?
  // Here: No
  in_ce.ending_entry = false;
}

GeometricReplay::GeometricReplay(unsigned int in_length_sec,
                                 unsigned int in_frames_per_sec) {
  this->threshhold_age_ = in_length_sec * in_frames_per_sec;
}

GeometricReplay::~GeometricReplay() {}

bool GeometricReplay::SaveFrame(entt::registry& in_registry) {
  // Loop over all entries with an id component
  entt::basic_view view = in_registry.view<IDComponent>();
  for (entt::entity entity : view) {
    IDComponent& id_c = in_registry.get<IDComponent>(entity);

    // Check if entity with that id has a channel
    bool unfound = true;
    for (unsigned int i = 0; i < this->channels_.size && unfound; i++) {
      if (id_c.id == this->channels_.at(i).object_id) {
        unfound = false;
        // If it does we check if a new interpolation point should be added
        // dependent on the object's type
        DataFrame* temp_df = nullptr;
        if (in_registry.has<PlayerComponent>(entity)) {
          // PLAYER CASE
          TransformComponent& transform_c =
              in_registry.get<TransformComponent>(entity);
          temp_df = new PlayerFrame(transform_c.position, transform_c.rotation,
                                    transform_c.scale);
        } else if (in_registry.has<PlayerComponent>(entity)) {
          // BALL CASE
          TransformComponent& transform_c =
              in_registry.get<TransformComponent>(entity);
          temp_df = new BallFrame(transform_c.position, transform_c.rotation,
                                  transform_c.scale);
        }

        // Compare last entry to what would be the current frame's
        if ((temp_df != nullptr) &&
            this->channels_.at(i).entries.back().data_ptr->ThresholdCheck(
                *temp_df)) {
          // IF true, save DataFrame
          ChannelEntry temp_ce;
          temp_ce.frame_number = this->current_frame_number_;
          temp_ce.data_ptr = temp_df;
          temp_ce.ending_entry = false;
          this->channels_.at(i).entries.push_back(temp_ce);
        }
      }
    }

    // If after looping through an object still hasn't been found
    // it should be added to its own channel
    if (unfound) {
      // Entry
      ChannelEntry temp_ce;
      this->FillChannelEntry(temp_ce, entity, in_registry);

      // Channel
      FrameChannel temp_fc;
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
      unsigned int age = this->current_frame_number_ -
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
  this->current_frame_number_++;

  return false;
}

bool GeometricReplay::LoadFrame(entt::registry& in_registry) {
  /*
                  WIP:
                  Load the next frame of the replay
                  Interpolate channels that do not hold a ChannelEntry
                  for current frame
  */

  return false;
}