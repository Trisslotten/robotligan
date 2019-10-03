#include "bit_pack.hpp"

#include <math.h>
#include <bitset>
#include <cstdio>
#include <vector>

// Private---------------------------------------------------------------------

// Public----------------------------------------------------------------------

BitPack::BitPack(unsigned int in_num_of_frames,
                 unsigned int in_bits_per_frame) {
  // Create a bitset capable of holding the amount
  // of data specified
  // 1 int = 4 bytes = 32 bits, thusly:

  this->num_of_bits_ = in_num_of_frames * in_bits_per_frame;
  unsigned int num_of_ints =
      (this->num_of_bits_ / 32) + 1;  // NTS: +1 to size to hold
                                      // potential remainder bits

  this->bits_ = new int[num_of_ints];

  // Fill the new array with only zeroes
  for (unsigned int i = 0; i < num_of_ints; i++) {
    this->bits_[i] = 0;
  }

  // Set counters to start
  this->next_bit_to_write_ = 0;
  this->next_bit_to_read_ = 0;
}

BitPack::~BitPack() {
  // Delete the array of bits
  if (this->bits_ != nullptr) {
    delete[] this->bits_;
  }
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

bool BitPack::IsWriteAtEnd() {
  // Return true if the end has been reached
  return (this->next_bit_to_write_ == this->num_of_bits_);
}

bool BitPack::IsReadAtEnd() {
  // Return true if the end has been reached
  return (this->next_bit_to_read_ == this->num_of_bits_);
}

unsigned int BitPack::GetNextWrittenBitIndex() const {
  // Send back the index of the last bit that was written
  return this->next_bit_to_write_;
}

unsigned int BitPack::GetNextReadBitIndex() const {
  // Send back the index of the last bit that was read
  return this->next_bit_to_read_;
}

bool BitPack::WriteBit(bool in_bit) {
  // Returns true if write successful.
  // Returns false if write failed (buffer end reached)

  // If end has been reached, do not write, return false
  if (this->next_bit_to_write_ == this->num_of_bits_) {
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

bool BitPack::WriteInt(unsigned int in_int, unsigned int in_bit_count) {
  // Returns true if write successful.
  // Returns false if write failed (buffer end reached)

  // Convert our integer to a binary value and write it
  // to the array using the WriteBit function.
  // If a write fails, quit and return false.
  // !!!---IMPORTANT---!!!
  // Writes the least worth bit first

  for (unsigned int i = 0; i < in_bit_count; i++) {
    if (!this->WriteBit((in_int % 2 == 1))) {
      return false;
    }
    in_int /= 2;
  }

  // Return write successful
  return true;
}

bool BitPack::WriteFloat32(float in_float) {
  // Returns true if write successful
  // Returns false if write failed (buffer end reached)

  // A 32-bit float is represented like the following in binary:
  //	|0|00000000|00000000000000000000000|
  // or simply put
  //	|1|8|23|
  // where the 1 bit tells us the sign
  // the 8 bits tells us the exponent
  // and the 23 bits tell us the fraction/mantissa

  // Cast the float to int
  uint32_t* int_ptr = reinterpret_cast<uint32_t*>(&in_float);

  // Put the bits in a bitset
  std::bitset<32> float_bits = *int_ptr;

  // Write 32 bits to the buffer
  for (unsigned int i = 0; i < 32; i++) {
    if (!this->WriteBit(float_bits[i])) {
      return false;
    }
  }

  // When finished return true
  return true;
}

bool BitPack::ReadBit() {
  // Returns the bit read from the buffer
  // at the buffers current read position

  // If reading counter would start going out of bounds
  // return 0 instead of continuing
  if (this->next_bit_to_read_ == this->num_of_bits_) {
    return false;
  }

  // Create an Integer 1	:	000000...00001
  // This value has 32 bits, so we can use it
  // to access the first bit in any other 32 bit
  // structure. By shifting it leftwards we can
  // pick which of the bits we want to check.
  // Example, leftsift 3	:	000000...01000
  bool ret_bit = this->bits_[this->next_bit_to_read_ / 32] &
                 ((unsigned int)1 << (this->next_bit_to_read_ % 32));

  // Increment reading counter
  this->next_bit_to_read_++;

  // Return the value
  return ret_bit;
}

unsigned int BitPack::ReadInt(unsigned int in_bit_count) {
  // Returns the integer stored on the number of
  // bits specified at the current read position

  int ret_num = 0;

  for (unsigned int i = 0; i < in_bit_count; i++) {
    if (this->ReadBit()) {
      ret_num += pow(2, i);
    }
  }

  return ret_num;
}

float BitPack::ReadFloat32() {
  // Returns the float stored on the 32 bits
  // at the current read location

  // Read 32 bits from the buffer
  std::bitset<32> float_bits;
  for (unsigned int i = 0; i < 32; i++) {
    float_bits[i] = this->ReadBit();
  }

  // Turn the bits to a 32 bit integer and then
  // cast it to a float
  uint32_t float_int = float_bits.to_ulong();
  float ret_num = *reinterpret_cast<float*>(&float_int);

  // Return the number
  return ret_num;
}

bool BitPack::SaveToFile(std::string in_path) {
  // Save the full buffer as integers in a file

  // Open the file to write binary to
  FILE* file_ptr;
  errno_t err = fopen_s(&file_ptr, in_path.c_str(), "wb");
  if (file_ptr == nullptr) {
    return false;
  }

  unsigned int num_of_ints = (this->num_of_bits_ / 32) + 1;

  // First write the number of integers in the file
  fwrite(&num_of_ints, sizeof(int), 1, file_ptr);

  // Then write all of the integers
  fwrite(this->bits_, sizeof(int), num_of_ints, file_ptr);

  // Lastly close the file
  fclose(file_ptr);
  return true;
}

bool BitPack::LoadFromFile(std::string in_path) {
  // Read the full buffer as intergers from a file

  // If there already is an array, delete it
  if (this->bits_ != nullptr) {
    delete[] this->bits_;
  }

  // Open the file to read binary from
  FILE* file_ptr;
  errno_t err = fopen_s(&file_ptr, in_path.c_str(), "rb");
  if (file_ptr == nullptr) {
    return false;
  }

  // Read the first integer and use it to allocate space
  // for the comming values
  unsigned int num_of_ints;
  fread(&num_of_ints, sizeof(int), 1, file_ptr);

  this->bits_ = new int[num_of_ints];

  // Then read all the following integers
  fread(this->bits_, sizeof(int), num_of_ints, file_ptr);

  // Lastly close the file
  fclose(file_ptr);
  return true;
}