#include "deterministic_replay.hpp"

// Private---------------------------------------------------------------------

void DeterministicReplay::WriteInputFrame(const std::bitset<10>& in_bitset,
                                          const float& in_x_value,
                                          const float& in_y_value) {
  // Format:
  //	{			:	(for every change 1+X bits are stored,
  //					where X is the number of bits needed
  //					to represent the largest index in
  // binary) 		1 bit	:	1, indicating a change 		X bits
  // : index int
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

void DeterministicReplay::ReadInputFrame(std::bitset<10>& in_bitset,
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

DeterministicReplay::DeterministicReplay(
    unsigned int in_num_of_frames, unsigned int in_num_of_keys,
    unsigned int in_snapshot_interval_frames) {
  // For one controlled entity we will require at maximum
  //	1 + B * (ceil(base2_log(N))+1) + 32 + 32
  // bits per frame, where B (buttons pressed/released) is
  // equal to N (all buttons).

  // Calculate how many bits we need to store
  // an int given how many keys we shall track
  this->bits_per_int_ = (unsigned int)ceil(log(in_num_of_keys) / log(2));

  // Calculate the maximum size that
  // might be required for one frame
  unsigned int bits_per_frame =
      (1 + in_num_of_keys * (this->bits_per_int_ + 1) + 64);

  // Create a bitpack capable of holding that size
  this->input_log_ = new BitPack(in_num_of_frames, bits_per_frame);

  // Set the values of the two helper bitsets
  this->last_input_state_ = std::bitset<10>("0000000000");
  this->last_output_state_ = std::bitset<10>("0000000000");

  //---

  // Calculate how many snapshots we will take
  // given the interval and number of frames
  // NTS: Add space for 1 extra
  unsigned int num_of_snapshots_ =
      (in_num_of_frames / in_snapshot_interval_frames) + 1;
  this->snapshot_interval_frames_ = in_snapshot_interval_frames;
  this->frames_since_last_snapshot_ = in_snapshot_interval_frames;

  // Allocate space for the snapshots
  this->registry_log_ = new RegPack(num_of_snapshots_);

  //---

  // Allocate space for the array linking bit and snapshot
  // indices together
  this->frame_indices_ = new unsigned int[num_of_snapshots_];
  this->curr_snapshot_index_ = 0;
}

DeterministicReplay::~DeterministicReplay() {
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

bool DeterministicReplay::SaveFrame(std::bitset<10>& in_bitset,
                                    float& in_x_value, float& in_y_value,
                                    entt::registry& in_registry) {
  // Returns true if end if buffer was reached

  // SNAPSHOT
  // Write regitry snapshot at certain intervals
  this->frames_since_last_snapshot_++;
  if (this->frames_since_last_snapshot_ >= this->snapshot_interval_frames_) {
    // If a snapshot was written we want to
    // couple it with the index from the bitpack
    unsigned int snapshot_index =
        this->registry_log_->GetNextWrittenSnapshotIndex();
    this->frame_indices_[snapshot_index] =
        this->input_log_->GetNextWrittenBitIndex();

    // Write the snapshot
    this->registry_log_->WriteSnapshot(in_registry);
    this->frames_since_last_snapshot_ = 0;
  }

  // BITPACK
  // Write input to bitpack each each frame
  this->WriteInputFrame(in_bitset, in_x_value, in_y_value);

  return this->input_log_->IsWriteAtEnd();
}

bool DeterministicReplay::LoadFrame(std::bitset<10>& in_bitset,
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