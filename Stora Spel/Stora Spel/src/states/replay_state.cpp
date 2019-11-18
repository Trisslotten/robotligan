#include "state.hpp"

#include "engine.hpp"

// Replay Stuff --------------------------------
// Private:

// Public:
void ReplayState::Init() {
  // Set this to be the current registry of engine
  this->engine_->SetCurrentRegistry(&this->replay_registry_);

  // WIP: Fix with other stuff here
}

void ReplayState::StartReplayMode() {
  // Do not start if already replaying
  if (replaying_) {
    return;
  }

  // Clear replay register
  this->replay_registry_.reset();

  // Set replaying to true
  this->replaying_ = true;

  // Get number of replays
  ClientReplayMachine* engine_rpm = this->engine_->GetReplayMachinePtr();
  this->num_of_replays_ = engine_rpm->NumberOfStoredReplays();
  this->replay_counter_ = 0;
  engine_rpm->SelectReplay(this->replay_counter_);
}

void ReplayState::PlayReplay() {
  if (!replaying_) {
    return;
  }

  ClientReplayMachine* engine_rpm = this->engine_->GetReplayMachinePtr();

  if (engine_rpm->LoadFrame(this->replay_registry_)) {
    // Once replay is done playing, clear the registry
    this->replay_registry_.reset();

    // And stop replaying
    this->replaying_ = false;
  }
}

// Replay Stuff --------------------------------