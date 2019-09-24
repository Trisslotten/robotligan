#include "replay_machine.hpp"

// Private---------------------------------------------------------------------

ReplayMachine::ReplayMachine() {
  // H
  // I
  this->input_log_ = NULL;
}

ReplayMachine::~ReplayMachine() {
  // B
  // Y
  // E
  if (this->input_log_ != NULL) {
    delete this->input_log_;
  }
}

// Public----------------------------------------------------------------------

void ReplayMachine::Init(unsigned int in_num_of_keys, unsigned int in_seconds) {
  // Create a BitPack capable of holding all inputs
  // we will allow logging (determines length of a replay).

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
  //	> We logivally deduce that B will be 0 most of the time and as such we end up with:
  //	>	1 * T  and  N * T
  //	> We now draw the conclusion (due to us knowing the nature of the game) that we have
  //	> more than 1 button. We also know there is (as of this being written) 20 ticks/second.
  //	> This means that on most seconds we are weighing storing 20 bits vs. storing (if we have
  //	> 8 different buttons) 160 bits. ALT 1 should thusly be the best choice.
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
  //	>	1 +  B * ceil(base2_log(N)) + 32 + 32
  //	> bits per frame, where B (buttons pressed/released) is
  //	> equal to N (all buttons).

  unsigned int max_num_of_frames = 20 * in_seconds;	//NTS: Tick rate is 20 frames ber second
  unsigned int max_bits_per_frame = 1 + in_num_of_keys * ceil(log(in_num_of_keys)/log(2));
  this->input_log_ = new BitPack(max_num_of_frames, max_bits_per_frame);
  
}