#ifndef FRAME_CHANNEL_HPP_
#define FRAME_CHANNEL_HPP_

#include <vector>

#include "data_frame.hpp"

class FrameChannel {
 private:
  struct ChannelEntry {
    unsigned int frame_number;
    DataFrame* frame_data_ptr;
  };

  std::vector<ChannelEntry> channel_;

 public:
  FrameChannel();
  ~FrameChannel();

  bool CheckThreshold();
};

#endif  // !FRAME_CHANNEL_HPP_
