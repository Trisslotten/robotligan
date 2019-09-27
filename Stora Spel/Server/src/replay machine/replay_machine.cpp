#include "replay_machine.hpp"

// Private---------------------------------------------------------------------

ReplayMachine::ReplayMachine() {
  // H
  // I
  this->input_log_ = nullptr;
  this->bits_per_int_ = 0;
}

ReplayMachine::~ReplayMachine() {
  // B
  // Y
  // E
  if (this->input_log_ != nullptr) {
    delete this->input_log_;
  }

}

// Public----------------------------------------------------------------------
ReplayMachine* ReplayMachine::Access() {
  // Return a pointer to this GlobalSettings object
  static ReplayMachine instance;
  return &instance;
}


void ReplayMachine::Init(unsigned int in_seconds, unsigned int in_ticks_per_second) {
  // Create a BitPack capable of holding the maximum
  // number of keys we will allow logging

  //---
  //
  // Float; Delta-time							:	0 bits	[Not needed since server tick-rate is a constant]
  //
  //---
  //
  //	N is number of keys
  //
  //	<<STORING KEY PRESSES ALT 1>>
  //	> Store if a key has been pressed and its index
  //	> This section can vary in total size from 1 to 1+ceil(base2_log(N)) bits
  //	+ Can store any number of keys easily
  //	+ Only stores the keys changed, not wasting space on unchanged keys
  //	- 
  //
  // Bool; has the value of a key changed?		:	1 bit	[any key]
  // {
  //	Int; the index of the value changed		:	X bits	[# of bits required depends on number of keys used
  //														in the game. Example: 3 bits can store 2^3 (8) keys.
  //														In general: X = ceil(base2_log(N)) ]
  //	Bool; has the value of a key changed?	:	1 bit	[next key]
  // }
  //
  //	<<STORING KEY PRESSES ALT 2>>
  //	> Store if each key has been pressed as one bit each
  //	> This section is as many bits as we have buttons
  //	+ Static size is easy to handle
  //	+ If two or more keys change at the same time we still only use 8 bits (lets assume we have 8 keys)
  //	- Even if values presses do not change we store N bits
  //
  // Bool; is a key pressed? xN					:	N bits	[one for each key]
  //
  //	<<DECISION>>
  //	> Assume the tick rate of the server is T.
  //	> Let B be numbers buttons pressed/released.
  //	> The question is when
  //	>	(1 + B * ceil(base2_log(N)) * T
  //	> surpasses N * T.
  //	> A probable assumption is that most seconds a player does not change a button.
  //	> Also, when the player does change a button that will occur on 1 or 2 frames
  //	> of that second (number of frames per second determined by the tick rate).
  //	> We logically deduce that B will be 0 most of the time and as such we end up with:
  //	>	1 * T  and  N * T
  //	> We now draw the conclusion (due to us knowing the nature of the game) that we have
  //	> more than 1 button. We also know there is (as of this being written) 20 ticks/second.
  //	> This means that on most seconds we are weighing storing 20 bits vs. storing (if we have
  //	> 10 different buttons) 200 bits. ALT 1 should thusly be the best choice.
  //
  //---
  //
  // Float; represents mouse's x-movement		: 32 bits	[ ] 
  // Float; represents mouse's y-movement		: 32 bits	[ ]
  //
  //	<<NTS>>
  //	> It might be posible to make these two optional, just as the key presses/releases
  //	> above, by storing the change instead, and if there is no change store nothing.
  //	> Two check bits would have to be added then, one before each float. If the bit
  //	> is set there is a 32 bit float upcoming.
  //
  //---
  //
  //---
  //
  //	<<RESULT>>
  //	> For one controlled entity we will require at maximum
  //	>	1 + B * ceil(base2_log(N)) + 32 + 32
  //	> bits per frame, where B (buttons pressed/released) is
  //	> equal to N (all buttons).

  unsigned int num_of_keys = this->last_input_state_.size();	// Note that number of keys is defined by the
																// static size we specified for the bitsets
  this->bits_per_int_ = (unsigned int)ceil(log(num_of_keys) / log(2));
  unsigned int max_num_of_frames =  in_ticks_per_second * in_seconds;
  unsigned int max_bits_per_frame = (1 + num_of_keys * this->bits_per_int_);  //NTS: Add floats
  this->input_log_ = new BitPack(max_num_of_frames, max_bits_per_frame);


}

void ReplayMachine::RecordKeys(const std::bitset<10>& in_bitset,
                               const float in_x_value, const float in_y_value) {
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
    } else {
	  //Otherwise note that no change has been made
      this->input_log_->WriteBit(0);
    }
  }
  
  // The float values that indicate angle/offset(?)
  // for mouse movements are always stored.
  this->input_log_->WriteFloat32(in_x_value);
  this->input_log_->WriteFloat32(in_y_value);
}

void ReplayMachine::TestFunction() {

  // Otherwise, create a BitPack.
  // Bitpack has space for 12 bits
  BitPack* test_pack = new BitPack(1, 12);

  // Write to the buffer										//#bits	:	How the buffer looks (index left-to-right)
  this->TestFunctionB('a', test_pack->WriteBit(0));				//1bit	:	0
  this->TestFunctionB('b', test_pack->WriteInt(5, 4));			//4bits	:	0-1010
  this->TestFunctionB('c1', test_pack->WriteBit(1));			//1bit	:	0-1010-1
  this->TestFunctionB('c2', test_pack->WriteBit(1));			//1bit	:	0-1010-1-1
  this->TestFunctionB('c3', test_pack->WriteBit(0));			//1bit	:	0-1010-1-1-0
  this->TestFunctionB('d', test_pack->WriteInt(3, 2));			//2bits	:	0-1010-1-1-0-11

  // Read the data saved as ints
  unsigned int nums[5] = {0, 0, 0, 0, 0};
  nums[0] = test_pack->ReadBit();	//1bit	:	0-1010-1-1-0-11	->	(0)		=	0
  nums[1] = test_pack->ReadInt(4);	//4bits	:	1010-1-1-0-11	->	(0101)	=	5
  nums[2] = test_pack->ReadInt(3);	//3bits	:	1-1-0-11		->	(011)	=	3
  nums[3] = test_pack->ReadBit();	//1bit	:	11				->	(1)		=	1
  nums[4] = test_pack->ReadBit();	//1bit	:	1				->	(1)		=	1

  // Output what has been read as a string.
  // It should look something like: 05311
  std::string out_str = "";
  for (unsigned int i = 0; i < 5; i++) {
    out_str += " " + std::to_string(nums[i]);
  }
  std::cout << "Exp: 0 5 3 1 1, Got: " << out_str << "\n";

  // By this point 10 of 12 bits should 
  // have been used. Write a 3 bit integer
  // then read 2 bits as int.
  // Write drops last bit	:	2bits	:	0010111011-11
  this->TestFunctionB('e', test_pack->WriteInt(7, 3));  // 3bits	:	[old]-111
														//(since there isn't enough space the last 1 will be dropped)
  int numB = test_pack->ReadInt(2);	//2bits	:	11	->	(11)	=	3

  //Output what was read
  std::cout << "Exp: 3, Got: " << std::to_string(numB) << "\n";
  
  //---

  // Now try reseting the read and write
  // counters in the bitpack.
  test_pack->ResetWrite();
  test_pack->ResetRead();

  // Write 1:s to 14 bits (2 fails expected)
  for (unsigned int i = 0; i < 14; i++) {
    this->TestFunctionB('f', test_pack->WriteBit(1));
  }

  // Read the contents as a 12 bit int (4095 expected)
  unsigned int numC = test_pack->ReadInt(12);

  // Output what has been read
  std::cout << "Exp: 4095, Got: " << std::to_string(numC) << "\n";

  //---

  // Write the BitPack to a file.
  test_pack->SaveToFile("src/replay machine/testreplay.bitpack");

  // Get rid of the test bitpack
  delete test_pack;

  // Create one that only holds 1 bit
  test_pack = new BitPack(1, 1);

  // Tell it to load from the file (it will allocate new space)
  test_pack->LoadFromFile("src/replay machine/testreplay.bitpack");

  // If we now read a 12 bit int we should once again get 4095
  // Read the contents as a 12 bit int (4095 expected)
  unsigned int numD = test_pack->ReadInt(12);
  std::cout << "Exp: 4095, Got: " << std::to_string(numD) << "\n";

  //---

  // Delete old pack
  // Create a new one that can hold at least 32 bits (a float)
  delete test_pack;
  test_pack = new BitPack(1, 32);

  std::cout << std::fixed;
  std::cout << "\n";

  float temp_float = 1024.6;
  this->TestFunctionB('g', test_pack->WriteFloat32(temp_float));
  std::cout << "Exp: " << temp_float << "\nGot: " << test_pack->ReadFloat32()
	        << "\n\n";
  test_pack->ResetWrite();
  test_pack->ResetRead();

  temp_float = 1024.625;
  this->TestFunctionB('h', test_pack->WriteFloat32(temp_float));
  std::cout << "Exp: " << temp_float << "\nGot: " << test_pack->ReadFloat32()
            << "\n\n";
  test_pack->ResetWrite();
  test_pack->ResetRead();

  temp_float = 1024.62533;
  this->TestFunctionB('i', test_pack->WriteFloat32(temp_float));
  std::cout << "Exp: " << temp_float << "\nGot: " << test_pack->ReadFloat32()
            << "\n\n";
  test_pack->ResetWrite();
  test_pack->ResetRead();

  // NTS: As we can see from the interior log-messages
  // from the WriteFloat32() function the modf() does not
  // make a clean cut and the binary write ends up writeing
  // two different values as the same.
  // NTS: Log-messages and the like are noted with //TEMP
  // in the BitPack-class for easy removal later.

  //---

  // Once done testing, pretend it never happened
  delete test_pack;
}

void ReplayMachine::TestFunctionB(char in_char, bool in_bool) {
  if (!in_bool) {
    std::cout << in_char << " : Write failed\n";
  }
  return;
}