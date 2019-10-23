#include "geometric_replay.hpp"

#include <map>

// Private---------------------------------------------------------------------

// Public----------------------------------------------------------------------

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

        /*
                WIP:
                Add switch case for object types(?)
                Add call to interpolation-point-threshold function
                If object should be added save frame number as well
        */
      }
    }

    // If after looping through an object still hasn't been found
    // it should be added to its own channel
    if (unfound) {
      /*
                      WIP:
                      Add new channel
      */
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
    if (this->channels_.at(i).entries.size() > 1) {				//(1.)
      unsigned int age = this->current_frame_number_ -
                         this->channels_.at(i).entries.at(1).frame_number;
      if (age > this->threshhold_age_) {						//(2.)
        if (this->channels_.at(i).entries.at(1).ending_entry) {	//(3.)
          this->channels_.erase(this->channels_.begin()+i, this->channels_.begin+i);
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