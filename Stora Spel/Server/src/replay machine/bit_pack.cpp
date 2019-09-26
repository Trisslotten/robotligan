#include "bit_pack.hpp"

#include <math.h>
#include <cstdio>
#include <vector>

#include <iostream>  //TEMP

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

bool BitPack::WriteBit(bool in_bit) {
  // Returns true if write successful.
  // Returns false if write failed (buffer end reached)

  // If end has been reached, do not write, return false
  if (this->next_bit_to_write_ >= this->num_of_bits_) {
    return false;
  }

  std::cout << in_bit; //TEMP

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

  // STEP 1:
  // Determine  if value is negative
  bool sign = (in_float < 0.0f);

  // STEP 2:
  // Split the float into a integer and fractional part
  float i_part;
  float f_part;

  f_part = modf(in_float, &i_part);

  std::cout << "Got: " << in_float << "\ni as float: " << i_part
            << "\ni as int: " << (int)i_part << "\nf as float: " << f_part
            << "\n"; //TEMP

  // STEP 3:
  // Convert the integer to a binary sequence
  std::vector<bool> binary_seq_i;
  for (int i = (int)i_part; i != 0; i /= 2) {
    // Check modulo two and add it to the vector
    // Note that the least worth bit lies first (0)
    binary_seq_i.push_back((i % 2) == 1);
  }

  // STEP 4:
  // Convert the fractional to a binary sequence
  // Note that we do this either until we have
  // 24 bits or we have "eaten" the entire float
  std::vector<bool> binary_seq_f;
  for (unsigned int i = 0; (i < 24) || (f_part != 0.0f); i++) {
    // Double the fractional part
    f_part *= 2.0f;
    // Check if it is equal to or exceeds 1
    if (f_part >= 1.0f) {
      // If it does, add a 1 to the vector
      binary_seq_f.push_back(1);
      // And subtract a 1 from the fractional part
      f_part -= 1.0f;
    } else {
      // Otherwise push back a 0
      binary_seq_f.push_back(0);
    }
  }

  // STEP 5:
  // Calculate mantissa
  // We now have two vectors describing the binary sequences
  // for the number backwards.
  // I.e. the last bit in each vector has the highest value.
  // Join the vectors together with the fraction at the front
  // and the integer at the back.
  std::vector<bool> mantissa;
  mantissa.insert(mantissa.end(), binary_seq_f.begin(), binary_seq_f.end());
  mantissa.insert(mantissa.end(), binary_seq_i.begin(), binary_seq_i.end());

  // The radix point (since the sequence is backwards)
  // would be the index the fractional sequence ends
  // after, that is to say: the length of that vector
  // Ex:	fffffff|iiii
  unsigned int radix_a = binary_seq_f.size();

  // Now shift the radix point so that the integer
  // value is 1. Do this by going from the back
  // until an index holding a 1 is found.
  int radix_b = 0;
  for (int i = (mantissa.size() - 1); i > -1; i--) {
    if (mantissa.at(i) == 1) {
      // Once found calculate the shift length.
      // Ex1:	fffffff|iiii
      //		fffffffii|ii	-> +2
      // Ex2:	fffffff|iiii
      //		fffff|ffiiii	-> -2
      radix_b = i - radix_a;
      // Then break the loop
      i = -1;
    }
  }

  // Trim away anything before the new radix point.
  // This includes the 1 as that one can be assumed
  // to exist.
  mantissa.erase(mantissa.begin() + radix_b, mantissa.end());

  // Now remove elements from the back of the number
  // (beginning of vector) to scale it down to 23 bit
  // precision, should it be over this.
  while (mantissa.size() > 23) {
    mantissa.erase(mantissa.begin(), mantissa.begin());
  }

  // If, reversly, the mantissa is shorter than 23
  // elements, add in 0:s at the back of the number
  while (mantissa.size() < 23) {
    mantissa.insert(mantissa.begin(), 0);
  }

  // STEP 6:
  // Calculate exponent
  // The exponent is the shift in radix point placement
  // plus the bias
  //	2^(k-1)-1
  // where k is the number of bits for the exponent.
  // For 32-bit floats k=8 which gives the bias 127
  unsigned int exponent = (radix_b - radix_a) + 127;

  // STEP 7:
  // Write the float to the buffer
  
  std::cout << "Program wrote float as: "; //TEMP

  // First write the sign-bit
  if (!this->WriteBit(sign)) {
    return false;
  }
  
  std::cout << "|"; //TEMP

  // Then write the exponent as 8 bits
  if (!this->WriteInt(exponent, 8)) {
    return false;
  }

  std::cout << "|"; //TEMP

  // Then write the mantissa as 23 bits
  // !!!---IMPORTANT---!!!
  // Writes the least worth bit first
  for (unsigned int i = 0; i < mantissa.size(); i++) {
    if (!this->WriteBit(mantissa.at(i))) {
      return false;
    }
  }

  std::cout << "\n"; //TEMP

  // When finished return true
  return true;
}

bool BitPack::ReadBit() {
  // Returns the bit read from the buffer
  // at the buffers current read position

  // Create an Integer 1	:	000000...00001
  // This value has 32 bits, so we can use it
  // to access the first bit in any other 32 bit
  // structure. By shifting it leftwards we can
  // pick which of the bits we want to check.
  // Example, leftsift 3	:	000000...01000
  bool bit = this->bits_[this->next_bit_to_read_ / 32] &
             ((unsigned int)1 << (this->next_bit_to_read_ % 32));

  // Increment reading counter
  this->next_bit_to_read_++;

  // If reading counter would start going out of bounds
  // set it to the valid last bit in the sequence
  if (this->next_bit_to_read_ >= this->num_of_bits_) {
    this->next_bit_to_read_ = this->num_of_bits_ - 1;
  }

  // Return the value
  return bit;
}

unsigned int BitPack::ReadInt(unsigned int in_bit_count) {
  // Returns the integer stored on the number of
  // bits specified at the current read position

  int num = 0;

  for (unsigned int i = 0; i < in_bit_count; i++) {
    if (this->ReadBit()) {
      num += pow(2, i);
    }
  }

  return num;
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