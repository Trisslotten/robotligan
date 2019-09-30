#ifndef REPLAY_MACHINE_HPP_
#define REPLAY_MACHINE_HPP_

#include <math.h>
#include <bitset>
#include <iostream>  //TEMP: For testing
#include <string>    //TEMP: For testing

#include "bit_pack.hpp"

class ReplayMachine {
 private:
  BitPack* input_log_;
  unsigned int bits_per_int_;
  std::bitset<10> last_input_state_;
  std::bitset<10> last_output_state_;

  // NTS:	Constructor and destructor
  //		both private members
  ReplayMachine();
  ~ReplayMachine();

  void WriteKeyFrame(const std::bitset<10>& in_bitset,
                       const float& in_x_value, const float& in_y_value);
  void ReadKeyFrame(std::bitset<10>& in_bitset, float& in_x_value,
                       float& in_y_value);

 public:
  // NTS:	Delete copy constructor
  //		and assignment operator
  ReplayMachine(ReplayMachine&) = delete;
  void operator=(ReplayMachine const&) = delete;

  static ReplayMachine* Access();
  void Init(unsigned int in_seconds, unsigned int in_ticks_per_second);
  

  void TestFunctionA();
  void TestFunctionA2(char in_char, bool in_bool);
  void TestFunctionB();
};

#endif  // !REPLAY_MACHINE_HPP_
