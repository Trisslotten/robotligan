#include "client_replay_machine.hpp"
#include "engine.hpp"

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
    this->stored_replays_.erase(this->stored_replays_.begin());
  }
}

void ClientReplayMachine::RecordFrame(entt::registry& in_registry) {
  // Tell primary replay to save the data
  this->primary_replay_->SaveFrame(in_registry);
}

void ClientReplayMachine::StoreReplay() {
  // Take a copy of the current state of the
  // primary replay and store it in the vector
  GeometricReplay* replay_to_save = this->primary_replay_->Clone();
  this->stored_replays_.push_back(replay_to_save);

  // Adjust the beginning of the stored vector
  // to lie at its threshold value
  this->stored_replays_.back()->ChannelCatchUp();
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
  this->ResetSelectedReplay();

  return true;
}

bool ClientReplayMachine::LoadFrame(entt::registry& in_registry) {
  // Returns true when replay is done

  // Return true if there is nothing in the stored_replays_ vector
  if (this->stored_replays_.empty()) {
    return true;
  }

  // Otherwise read a frame into the registry
  // LoadFrame() returns true if the read index has
  // caught up with the write index
  bool load_result = this->stored_replays_.at(this->selected_replay_index_)
                         ->LoadFrame(in_registry);

  return load_result;
}

void ClientReplayMachine::ResetSelectedReplay() {
  // Resets current replay to start
  this->stored_replays_.at(this->selected_replay_index_)->SetReadFrameToStart();
}

std::string ClientReplayMachine::GetSelectedReplayStringTree() {
  if (this->stored_replays_.empty()) {
    return "ERROR: There are no stored replays";
  }

  return this->stored_replays_.at(this->selected_replay_index_)
      ->GetGeometricReplayTree();
}

std::string ClientReplayMachine::GetSelectedReplayStringState() {
  if (this->stored_replays_.empty()) {
    return "ERROR: There are no stored replays";
  }

  return this->stored_replays_.at(this->selected_replay_index_)
      ->GetStateOfReplay();
}

void ClientReplayMachine::ReceiveGameEvent(GameEvent event) {
  if (this->engine_->IsRecording()) {
    primary_replay_->ReceiveGameEvent(event);
  }
}
