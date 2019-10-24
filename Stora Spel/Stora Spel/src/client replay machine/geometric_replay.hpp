#ifndef GEOMETRIC_REPLAY_HPP_
#define GEOMETRIC_REPLAY_HPP_

#include <vector>

#include <entity/registry.hpp>
#include <shared/id_component.hpp>

#include "data_frame.hpp"

class GeometricReplay {
 private:
  struct ChannelEntry {
    unsigned int frame_number = -1;
    DataFrame* data_ptr = nullptr;
    bool ending_entry = false;

    ~ChannelEntry() {
      if (data_ptr != nullptr) {
        delete data_ptr;
      }
    }
  };

  struct FrameChannel {
    EntityID object_id;
    std::vector<ChannelEntry> entries;
  };

  std::vector<FrameChannel> channels_;
  unsigned int threshhold_age_;
  unsigned int current_frame_number_ = 0;

  void FillChannelEntry(ChannelEntry& in_ce, entt::entity& in_entity, entt::registry& in_registry);

 public:
  GeometricReplay(unsigned int in_length_sec, unsigned int in_frames_per_sec);
  ~GeometricReplay();

  // NTS:	Delete copy constructor
  //		and assignment operator
  GeometricReplay(GeometricReplay&) = delete;
  void operator=(GeometricReplay const&) = delete;

  bool SaveFrame(entt::registry& in_registry);
  bool LoadFrame(entt::registry& in_registry);
};

#endif  // !GEOMETRIC_REPLAY_HPP_
