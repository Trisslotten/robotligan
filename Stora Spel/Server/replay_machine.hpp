#ifndef REPLAY_MACHINE_HPP_
#define REPLAY_MACHINE_HPP_

#include <math.h>

#include "bit_pack.hpp"

class ReplayMachine {
 private:
  BitPack* input_log_;

  // NTS:	Constructor and destructor
  //		both private members
  ReplayMachine();
  ~ReplayMachine();

 public:
  void Init(unsigned int in_num_of_keys, unsigned int in_seconds);
};

#endif  // !REPLAY_MACHINE_HPP_
