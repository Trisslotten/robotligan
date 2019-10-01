#include "replay_machine.hpp"

// Private---------------------------------------------------------------------

ReplayMachine::ReplayMachine() {
  // H
  // I
  this->input_log_ = nullptr;
  this->bits_per_int_ = 0;
  this->registry_log_ = nullptr;
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

}

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
ReplayMachine* ReplayMachine::Access() {
  // Return a pointer to this GlobalSettings object
  static ReplayMachine instance;
  return &instance;
}

void ReplayMachine::Init(unsigned int in_seconds,
                         unsigned int in_ticks_per_second,
                         int in_snapshot_interval_seconds) {
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
  //	> This section can vary in total size from 1 to 1+ceil(base2_log(N))+1 bits
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
  //	>	1 + B * (ceil(base2_log(N))+1) + 32 + 32
  //	> bits per frame, where B (buttons pressed/released) is
  //	> equal to N (all buttons).

  //---

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

  // Do not bother doing any of this if the supplied value
  // is negative
  if (in_snapshot_interval_seconds < 0) {
    return;
  }

  // Calculate how many snapshots we will be able to squeeze in
  // based on seconds the replay records and interval of snapshots
  // Allocate space for one extra, the last frame of the game
  unsigned int num_of_snapshots_ = (in_seconds / in_snapshot_interval_seconds) + 1;

  // Allocate space for the snapshots
  this->registry_log_ = new RegPack(num_of_snapshots_);
}

void ReplayMachine::TestFunctionA() {

  // Otherwise, create a BitPack.
  // Bitpack has space for 12 bits
  BitPack* test_pack = new BitPack(1, 12);

  // Write to the buffer										//#bits	:	How the buffer looks (index left-to-right)
  this->TestFunctionA2('a', test_pack->WriteBit(0));				//1bit	:	0
  this->TestFunctionA2('b', test_pack->WriteInt(5, 4));			//4bits	:	0-1010
  this->TestFunctionA2('c1', test_pack->WriteBit(1));			//1bit	:	0-1010-1
  this->TestFunctionA2('c2', test_pack->WriteBit(1));			//1bit	:	0-1010-1-1
  this->TestFunctionA2('c3', test_pack->WriteBit(0));			//1bit	:	0-1010-1-1-0
  this->TestFunctionA2('d', test_pack->WriteInt(3, 2));			//2bits	:	0-1010-1-1-0-11

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
  this->TestFunctionA2('e', test_pack->WriteInt(7, 3));  // 3bits	:	[old]-111
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
    this->TestFunctionA2('f', test_pack->WriteBit(1));
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
  this->TestFunctionA2('g', test_pack->WriteFloat32(temp_float));
  std::cout << "Exp: " << temp_float << "\nGot: " << test_pack->ReadFloat32()
	        << "\n\n";
  test_pack->ResetWrite();
  test_pack->ResetRead();

  temp_float = 1024.625;
  this->TestFunctionA2('h', test_pack->WriteFloat32(temp_float));
  std::cout << "Exp: " << temp_float << "\nGot: " << test_pack->ReadFloat32()
            << "\n\n";
  test_pack->ResetWrite();
  test_pack->ResetRead();

  temp_float = 1024.62533;
  this->TestFunctionA2('i', test_pack->WriteFloat32(temp_float));
  std::cout << "Exp: " << temp_float << "\nGot: " << test_pack->ReadFloat32()
            << "\n\n";
  test_pack->ResetWrite();
  test_pack->ResetRead();

  //---

  // Once done testing, pretend it never happened
  delete test_pack;
}

void ReplayMachine::TestFunctionA2(char in_char, bool in_bool) {
  if (!in_bool) {
    std::cout << in_char << " : Write failed\n";
  }
  return;
}

void ReplayMachine::TestFunctionB() {
  
  // Set cout to show more decimals
  std::cout << std::fixed;
  std::cout << "\n";

  // Delete the local input_log_ if it isn't null
  // Then initiate it to hold 5 entries
  if (this->input_log_ != nullptr) {
    delete this->input_log_;
  }
  this->Init(1, 5);

  // Create some dummy input values
  std::bitset<10> frame_bits[5];
  frame_bits[0] = std::bitset<10>("1000000000");
  frame_bits[1] = std::bitset<10>("1100000000");
  frame_bits[2] = std::bitset<10>("1100001000");
  frame_bits[3] = std::bitset<10>("0000001000");
  frame_bits[4] = std::bitset<10>("1111110111");
  float frame_floats[10] = {
	  1.00, 1.00,		//f1
      1.24, 0.78,		//f2
      2.305, 0.67821,	//f3
      2.0, -0.00635,	//f4
      -4.000089723, 1.2	//f5
  };

  // Input the five dummy frames to the log
  for (unsigned int i = 0; i < 5; i++) {
    this->WriteInputFrame(frame_bits[i], frame_floats[2*i], frame_floats[(2*i)+1]);
  }

  std::cout << "---------\n\n";

  // Now retrive each frame and compare it to the
  // dummy inputs
  std::bitset<10> retriever_bits;
  float retriever_floats[2] = {0, 0};
  
  for (unsigned int i = 0; i < 5; i++) {
    this->ReadInputFrame(retriever_bits, retriever_floats[0], retriever_floats[1]);
    std::cout << "I: " << frame_bits[i] << "\n";
    std::cout << "O: " << retriever_bits << "\n";
    std::cout << "I: " << frame_floats[2*i] << " : " << frame_floats[2*i+1] << "\n";
    std::cout << "O: " << retriever_floats[0] << " : " << retriever_floats[1] << "\n\n";
  }

  //---

  // Once done testing, pretend it never happened
  delete this->input_log_;
  this->input_log_ = nullptr;
}

entt::registry ReplayMachine::TestFunctionC(entt::registry& in_registry) {
  // Take a registry from outside
  // Note that his class knows of no components
  // so to properly test it define a component outside
  
  // Initiate
  this->Init(1, 1, 1);
  
  // Clone the registry into the local
  this->registry_log_->WriteSnapshot(in_registry);

  // Create an empty register locally
  // and load the stored data into it
  entt::registry loc_reg;
  this->registry_log_->ReadSnapshot(loc_reg);

  //Return the registry
  return loc_reg;
}