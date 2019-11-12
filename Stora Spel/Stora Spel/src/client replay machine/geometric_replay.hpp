#ifndef GEOMETRIC_REPLAY_HPP_
#define GEOMETRIC_REPLAY_HPP_

#include <string>
#include <vector>

#include <entity/registry.hpp>
#include <shared/id_component.hpp>

#include "data_frame.hpp"

enum ReplayObjectType {
  REPLAY_PLAYER = 0,  // Start
  REPLAY_BALL,
  REPLAY_PICKUP,
  REPLAY_SHOT,
  NUM_OF_REPLAY_OBJECT_TYPES  // End
};

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
        this->data_ptr = in_ce.data_ptr->Clone();
      }
      this->ending_entry = in_ce.ending_entry;
    }

    ~ChannelEntry() {
      if (data_ptr != nullptr) {
        delete data_ptr;
      }
    }

    void operator=(ChannelEntry const& rhs) {
      this->frame_number = rhs.frame_number;
      this->data_ptr = nullptr;
      if (rhs.data_ptr != nullptr) {
        this->data_ptr = rhs.data_ptr->Clone();
      }
      this->ending_entry = rhs.ending_entry;
    }
  };

  struct FrameChannel {
    ReplayObjectType object_type;
    EntityID object_id;
    std::vector<ChannelEntry> entries;
    unsigned int index_a = 0;
    unsigned int index_b = 0;
  };

  ReplayObjectType IdentifyEntity(entt::entity& in_entity,
                                  entt::registry& in_registry);

  void FillChannelEntry(ChannelEntry& in_ce, entt::entity& in_entity,
                        entt::registry& in_registry);
  DataFrame* PolymorphIntoDataFrame(entt::entity& in_entity,
                                    entt::registry& in_registry);

  DataFrame* InterpolateDataFrame(unsigned int in_channel_index,
                                  unsigned int in_entry_index_a,
                                  unsigned int in_entry_index_b,
                                  unsigned int in_target_frame_num);

  void InterpolateEntityData(unsigned int in_channel_index,
                             entt::entity& in_entity,
                             entt::registry& in_registry);
  void DepolymorphFromDataframe(DataFrame* in_df_ptr, ReplayObjectType in_type,
                                entt::entity& in_entity,
                                entt::registry& in_registry);

  void CreateEntityFromChannel(unsigned int in_channel_index,
                               entt::registry& in_registry);

 protected:
  std::vector<FrameChannel> channels_;
  unsigned int threshhold_age_;
  unsigned int current_frame_number_write_ = 0;
  unsigned int current_frame_number_read_ = 0;

  GeometricReplay();

 public:
  GeometricReplay(unsigned int in_replay_length_sec,
                  unsigned int in_frames_per_sec);
  ~GeometricReplay();

  // NTS:	Delete copy constructor
  //		and assignment operator
  GeometricReplay(GeometricReplay&) = delete;
  void operator=(GeometricReplay const&) = delete;

  GeometricReplay* Clone();

  bool SaveFrame(entt::registry& in_registry);
  bool LoadFrame(entt::registry& in_registry);

  // void SetWriteFrame(unsigned int in_frame_number);
  // NTS: Do not use this (^^^) function. The only place where
  // the write-frame number is set is in the SaveFrame() function
  // which means that the write-frame number can tell us the age
  // of the replay

  void SetReadFrameToStart();

  void ChannelCatchUp();

  std::string GetGeometricReplayTree();
  std::string GetStateOfReplay();
};

#endif  // !GEOMETRIC_REPLAY_HPP_
