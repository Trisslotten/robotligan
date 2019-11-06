#include "client_replay_machine.hpp"

// Private---------------------------------------------------------------------

// Public----------------------------------------------------------------------

ClientReplayMachine::ClientReplayMachine(unsigned int in_replay_length_sec,
                                         unsigned int in_frames_per_sec) {
  // Set data for the length and frame rate of
  // replays created by this replay machine
  this->replay_length_sec_ = in_replay_length_sec;
  this->replay_frames_per_sec_ = in_frames_per_sec;

  // Create a primary replay
  this->primary_replay_ =
      new GeometricReplay(in_replay_length_sec, in_frames_per_sec);

  // ---
  this->selected_replay_index_ = 0;

  // Reset and pause the timer
  this->replay_timer_.Restart();
  this->replay_timer_.Pause();
}

ClientReplayMachine::~ClientReplayMachine() {
  if (this->primary_replay_ != nullptr) {
    delete this->primary_replay_;
    this->primary_replay_ = nullptr;
  }

  while (!(this->stored_replays_.empty())) {
    if (this->stored_replays_.at(0) != nullptr) {
      delete this->stored_replays_.at(0);
    }
    this->stored_replays_.erase(this->stored_replays_.begin(),
                                this->stored_replays_.begin());
  }
}

void ClientReplayMachine::RecordFrame(entt::registry& in_registry) {
  // Tell primary replay to save the data
  this->primary_replay_->SaveFrame(in_registry);
}

void ClientReplayMachine::StoreReplay() {
  // Take a copy of the current state of the
  // primary replay and store it in the vector
  this->stored_replays_.push_back(this->primary_replay_->Clone());
}

unsigned int ClientReplayMachine::NumberOfStoredReplays() const {
  return this->stored_replays_.size();
}

int ClientReplayMachine::CurrentlySelectedReplay() const {
  // Returns -1 if there are no stored replays
  if (this->stored_replays_.empty()) {
    return -1;
  }
  return this->selected_replay_index_;
}

bool ClientReplayMachine::SelectReplay(unsigned int in_index) {
  // If the index is out of scope return false
  if (in_index > this->stored_replays_.size()) {
    return false;
  }

  // Otherwise jump to the given index
  this->selected_replay_index_ = in_index;

  // Set its read to the start
  this->stored_replays_.at(this->selected_replay_index_)->SetReadFrame(0);
  
  // Reset and pause the timer
  this->replay_timer_.Restart();
  this->replay_timer_.Pause();

  return true;
}

bool ClientReplayMachine::LoadFrame(entt::registry& in_registry) {
  // Returns true when replay is done

  // Return true if there is nothing in the stored_replays_ vector
  if (this->stored_replays_.empty()) {
    return true;
  }

  //If the timer has yet to be started, start it
  if (this->replay_timer_.Elapsed() == 0) {
    this->replay_timer_.Resume();
  }

  //If the timer shows we have run out of time, return
  if (this->replay_timer_.Elapsed() >= (double)this->replay_length_sec_) {
    this->replay_timer_.Pause();
    return true;
  }

  // Otherwise read a frame into the registry
  // NTS: GeometricReplay::LoadFrame() currently always returns true
  bool load_result = this->stored_replays_.at(this->selected_replay_index_)
                         ->LoadFrame(in_registry);

  return false;
}