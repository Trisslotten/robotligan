#include "state.hpp"

#include "engine.hpp"

// Replay Stuff --------------------------------
// Private:

// Public:
void ReplayState::Init() {
  // Set this to be the current registry of engine
  this->engine_->SetCurrentRegistry(&this->replay_registry_);

  //WIP: Fix with other stuff here

}

void ReplayState::StartReplayMode() {
  if (replaying_) {
    return;
  }

  //Clear replay register
  this->replay_registry_.reset();

  // WIP: WORKING HERE
  replaying_ = true;
  replay_counter_ = replay_machine_->NumberOfStoredReplays() - 1;
  replay_machine_->SelectReplay(replay_counter_);
}

void ReplayState::PlayReplay() {
  if (!replaying_) {
    return;
  }

  ClientReplayMachine* engine_rpm = this->engine_->GetReplayMachinePtr();

  if (engine_rpm->LoadFrame(this->replay_registry_)) {
    //Once replay is done playing, clear the registry
    this->replay_registry_.reset();

	//And stop replaying
    this->replaying_ = false;
  }
}

// Replay Stuff --------------------------------