#ifndef REPLAY_MACHINE_HPP_
#define REPLAY_MACHINE_HPP_

#include <math.h>
#include <string>	//TEMP: For testing
#include <iostream>	//TEMP: For testing

#include "bit_pack.hpp"

class ReplayMachine {
 private:
  BitPack* input_log_;

  // NTS:	Constructor and destructor
  //		both private members
  ReplayMachine();
  ~ReplayMachine();

 public:
  // NTS:	Delete copy constructor
  //		and assignment operator
  ReplayMachine(ReplayMachine&) = delete;
  void operator=(ReplayMachine const&) = delete;

  static ReplayMachine* Access();
  void Init(unsigned int in_num_of_keys, unsigned int in_seconds);

  void TestFunction();
  void TestFunctionB(char in_char, bool in_bool);
};

#endif  // !REPLAY_MACHINE_HPP_
