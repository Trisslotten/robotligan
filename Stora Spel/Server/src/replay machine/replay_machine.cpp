#include "replay_machine.hpp"

// Private---------------------------------------------------------------------

void ReplayMachine::WriteInputFrame(const std::bitset<10>& in_bitset,
                                    const float& in_x_value,
                                    const float& in_y_value) {

  // Format:	
  //	{			:	(for every change 1+X bits are stored,
  //					where X is the number of bits needed
  //					to represent the largest index in binary)
  //		1 bit	:	1, indicating a change
  //		X bits	:	index int
  //	}
  //	1 bit	:	0, indicating the end of the first section
  //	32 bits	:	x_value float
  //	32 bits	:	y_value float

  // Loop over the bitset
  for (unsigned int i = 0; i < in_bitset.size(); i++) {
    // IF its bit is different from last input state's
    if (in_bitset[i] != this->last_input_state_[i]) {
      
	  // Write that there has been a change
      this->input_log_->WriteBit(1);
      
	  // At index
      this->input_log_->WriteInt(i, this->bits_per_int_);
      
	  // Then flip that bit in the local bitset
      this->last_input_state_.flip(i);
    }
  }
  // Once everything has been looped over, no more changes occur
  // and we write a 0 to indicate the end of this section
  this->input_log_->WriteBit(0);

  // The float values that indicate angle/offset(?)
  // for mouse movements are always stored.
  this->input_log_->WriteFloat32(in_x_value);
  this->input_log_->WriteFloat32(in_y_value);
}

void ReplayMachine::ReadInputFrame(std::bitset<10>& in_bitset,
                                    float& in_x_value, float& in_y_value) {
  // Read the first bit. If it is 1 a change has occured
  // and the following bits specify the index in the bitset.
  unsigned int counter = 0;
  unsigned int index = 0;
  while (this->input_log_->ReadBit() && counter < 10) {
    // Get the number telling of the index where the change occured
    index = this->input_log_->ReadInt(this->bits_per_int_);
    // In the output bitset flip that bit
    this->last_output_state_.flip(index);
    // Increment counter
    counter++;
  }

  // Once either there has been no more change or 10 values has
  // been read, read two 32 bit floats from the buffer
  float x_float = this->input_log_->ReadFloat32();
  float y_float = this->input_log_->ReadFloat32();

  // Finally put the values into the provided references
  in_bitset = this->last_output_state_;
  in_x_value = x_float;
  in_y_value = y_float;
}


// Public----------------------------------------------------------------------
ReplayMachine::ReplayMachine() {
  // H
  // I
  this->input_log_ = nullptr;
  this->bits_per_int_ = 0;
  this->registry_log_ = nullptr;
  this->snapshot_interval_seconds_ = 0.0f;
  this->time_since_last_snapshot_ = 0.0f;
  this->frame_indices_ = nullptr;
  this->curr_snapshot_index_ = 0;
}

ReplayMachine::~ReplayMachine() {
  // B
  // Y
  // E
  if (this->input_log_ != nullptr) {
    delete this->input_log_;
  }
  if (this->registry_log_ != nullptr) {
    delete this->registry_log_;
  }
  if (this->frame_indices_ != nullptr) {
    delete[] this->frame_indices_;
  }
}

void ReplayMachine::Init(unsigned int in_seconds,
                         unsigned int in_ticks_per_second,
                         float in_snapshot_interval_seconds) {
  // Create a BitPack capable of holding the maximum
  // number of keys we will allow logging

  // For one controlled entity we will require at maximum
  //	1 + B * (ceil(base2_log(N))+1) + 32 + 32
  // bits per frame, where B (buttons pressed/released) is
  // equal to N (all buttons).

  // Calculate number of frame that will be recorded
  // as well as the number of bits each frame might need
  unsigned int num_of_keys = this->last_input_state_.size();	// Note that number of keys is defined by the
																// static size we specified for the bitsets
  this->bits_per_int_ = (unsigned int)ceil(log(num_of_keys) / log(2));
  unsigned int max_num_of_frames =  in_ticks_per_second * in_seconds;
  unsigned int max_bits_per_frame = (1 + num_of_keys * (this->bits_per_int_ + 1) + 64);
  
  // Allocate space for the input log
  this->input_log_ = new BitPack(max_num_of_frames, max_bits_per_frame);

  // Set the values of the two helper bitsets
  this->last_input_state_ = std::bitset<10>("0000000000");
  this->last_output_state_ = std::bitset<10>("0000000000");

  //---

  // Calculate how many snapshots we will be able to squeeze in
  // based on seconds the replay records and interval of snapshots
  // Allocate space for one extra, the last frame of the game
  unsigned int num_of_snapshots_ = (in_seconds / in_snapshot_interval_seconds) + 1;
  this->snapshot_interval_seconds_ = in_snapshot_interval_seconds;
  this->time_since_last_snapshot_ = in_snapshot_interval_seconds;

  // Allocate space for the snapshots
  this->registry_log_ = new RegPack(num_of_snapshots_);

  // Allocate space for the array linking bit and snapshot
  // indices together
  this->frame_indices_ = new unsigned int[num_of_snapshots_];
}

bool ReplayMachine::SaveReplayFrame(std::bitset<10>& in_bitset,
                                    float& in_x_value, float& in_y_value,
                                    entt::registry& in_registry,
                                    const float& in_dt) {
  // Returns true if end if buffer was reached

  //SNAPSHOT
  // Write regitry snapshot at certain intervals
  this->time_since_last_snapshot_ += in_dt;
  if (this->time_since_last_snapshot_ >= this->snapshot_interval_seconds_) {
    // If a snapshot was written we want to
    // couple it with the index from the bitpack
    unsigned int snapshot_index =
        this->registry_log_->GetNextWrittenSnapshotIndex();
    this->frame_indices_[snapshot_index] =
        this->input_log_->GetNextWrittenBitIndex();

	// Write the snapshot  
	this->registry_log_->WriteSnapshot(in_registry);
    this->time_since_last_snapshot_ = 0.0f;
  }
  
  //BITPACK
  // Write input to bitpack each each frame
  this->WriteInputFrame(in_bitset, in_x_value, in_y_value);

  return this->input_log_->IsWriteAtEnd();
}

bool ReplayMachine::LoadReplayFrame(std::bitset<10>& in_bitset,
                                    float& in_x_value, float& in_y_value,
                                    entt::registry& in_registry) {
  // Returns true if end if buffer was reached

  // Get what bit index is about to be read from
  unsigned int next_bit_index = this->input_log_->GetNextReadBitIndex();

  // If the read bit index corresponds to the one saved
  // in the array, read a registry snapshot
  if (next_bit_index == this->frame_indices_[this->curr_snapshot_index_]) {
    this->registry_log_->ReadSnapshot(in_registry);
    this->curr_snapshot_index_++;
  }

  // Get replay data and put it into the given references
  // Read the bitpack each frame
  this->ReadInputFrame(in_bitset, in_x_value, in_y_value);

  return this->input_log_->IsReadAtEnd();
}
