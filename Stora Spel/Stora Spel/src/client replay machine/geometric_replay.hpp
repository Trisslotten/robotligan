#ifndef GEOMETRIC_REPLAY_HPP_
#define GEOMETRIC_REPLAY_HPP_

#include <vector>

#include <entity/registry.hpp>
#include <shared/id_component.hpp>

#include "frame_channel.hpp"

class GeometricReplay {
 private:

  struct ChannelEntry {
    unsigned int frame_number;
    DataFrame* data_ptr;
  };

  struct FrameChannel {
    EntityID object_id;
    std::vector<ChannelEntry> entries;
  };

  std::vector<FrameChannel> channels_;

 public:
  GeometricReplay();
  ~GeometricReplay();

  bool SaveFrame(entt::registry& in_registry);
  bool LoadFrame(entt::registry& in_registry);
};

#endif  // !GEOMETRIC_REPLAY_HPP_
