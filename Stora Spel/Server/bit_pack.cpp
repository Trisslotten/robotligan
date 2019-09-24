#include "bit_pack.hpp"

// Private---------------------------------------------------------------------

// Public----------------------------------------------------------------------

BitPack::BitPack(unsigned int in_num_of_frames,
                 unsigned int in_bits_per_frame) {
  // Create a bitset capable of holding the amount
  // of data specified
  // 1 int = 4 bytes = 32 bits, thusly:

  unsigned int num_of_bits = in_num_of_frames * in_bits_per_frame;
  unsigned int num_of_ints =
      (num_of_bits / 32) + 1;  // NTS: +1 to size to hold
                               // potential remainder bits

  this->bits_ = new int[num_of_ints];

  this->arr_size_ = num_of_bits;

  this->next_bit_to_write_ = 0;
  this->next_bit_to_read_ = 0;
}

BitPack::~BitPack() {
  // Delete the array of bits
  delete[] this->bits_;
}

void BitPack::ResetWrite() {
  // Sets the writing index back to 0.
  // This means next time we write we will overwrite
  // things already put into the sequence
  this->next_bit_to_write_ = 0;
}

void BitPack::ResetRead() {
  // Sets the reading index back to 0.
  // This means next time we read we will read
  // from the start
  this->next_bit_to_read_ = 0;
}

bool BitPack::WriteBit(bool in_bit) {
  // Returns true if write successful.
  // Returns false if write failed (buffer end reached)

  // If end has been reached, do not write, return false
  if (this->next_bit_to_write_ / 32 >= this->arr_size_) {
    return false;
  }

  // Determine what array index we are on
  // as well as which bit to alter
  unsigned int arr_i = this->next_bit_to_write_ / 32;
  unsigned int bit_i = this->next_bit_to_write_ % 32;

  // Set bit
  this->bits_[arr_i] |= in_bit << (bit_i);

  // Increment writing counter
  this->next_bit_to_write_++;

  // Return write successful
  return true;
}

bool BitPack::WriteInteger(unsigned int in_int, unsigned int in_bit_count) {
  // Returns true if write successful.
  // Returns false if write failed (buffer end reached)

  // Check if the integer needs more space than we have
  // Loop from the first bit index we would write up as
  // many as we have been told the integer should take up
  for (unsigned int i = this->next_bit_to_write_;
       i < this->next_bit_to_write_ + in_bit_count; i++) {
    // If the array index would ever surpass the size
    // of the array, return false.
    if (i / 32 >= this->arr_size_) {
      return false;
    }
  }

  // If we are not going out of scope convert our integer to
  // a binary value and write it to the array using the
  // WriteBit function.

  // !!!---IMPORTANT---!!!
  // Writes the least worth bit first

  for (unsigned int i = 0; i < in_bit_count; i++) {
    this->WriteBit((in_int % 2 == 1));
    in_int /= 2;
  }

  // Return write successful
  return true;
}

bool BitPack::ReadBit() const {
  //
  //
}

unsigned int BitPack::ReadInt(unsigned int in_bit_count) const {
  //
  //
}