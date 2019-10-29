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

	ChannelEntry() {}

    ChannelEntry(const ChannelEntry& in_ce) {
      this->frame_number = in_ce.frame_number;
      this->data_ptr = nullptr;
      if (in_ce.data_ptr != nullptr) {
        this->data_ptr = this->data_ptr->Clone();
	  }
      this->ending_entry = in_ce.ending_entry;
    }

    ~ChannelEntry() {
      if (data_ptr != nullptr) {
        delete data_ptr;
      }
    }

  };

  struct FrameChannel {
    EntityID object_id;
    std::vector<ChannelEntry> entries;
    unsigned int index_a = 0;
    unsigned int index_b = 0;
  };

  std::vector<FrameChannel> channels_;
  unsigned int threshhold_age_;
  unsigned int current_frame_number_write_ = 0;
  unsigned int current_frame_number_read_ = 0;

  void FillChannelEntry(ChannelEntry& in_ce, entt::entity& in_entity,
                        entt::registry& in_registry);
  DataFrame* PolymorphIntoDataFrame(entt::entity& in_entity,
                                    entt::registry& in_registry);

  void InterpolateEntityData(unsigned int in_channel_index,
                             entt::entity& in_entity,
                             entt::registry& in_registry);
  void DepolymorphFromDataframe(DataFrame* in_df_ptr, entt::entity& in_entity,
                                entt::registry& in_registry);

  void CreateEntityFromChannel(unsigned int in_channel_index,
                               entt::registry& in_registry);

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
