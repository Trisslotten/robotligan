#include "reg_pack.hpp"

// Private---------------------------------------------------------------------

// Public----------------------------------------------------------------------

RegPack::RegPack(unsigned int in_num_of_snapshots) {
  // H
  // A
  // I
  this->snapshots_ = new entt::registry[in_num_of_snapshots];
  this->num_of_snapshots_ = in_num_of_snapshots;

  this->next_snapshot_to_write_ = 0;
  this->next_snapshot_to_read_ = 0;
}

RegPack::~RegPack() {
  // B
  // Y
  // E
  delete[] this->snapshots_;
}

unsigned int RegPack::GetNextWrittenSnapshotIndex() const {
  // Send back the index of the last snapshot that was written
  return this->next_snapshot_to_write_;
}

void RegPack::ResetWrite() {
  // Sets the writing index back to 0.
  this->next_snapshot_to_write_ = 0;
}

void RegPack::ResetRead() {
  // Sets the reading index back to 0.
  this->next_snapshot_to_read_ = 0;
}

bool RegPack::WriteSnapshot(entt::registry& in_registry) {
  // If end reached return false
  if (this->next_snapshot_to_write_ >= this->num_of_snapshots_) {
    return false;
  }
  // Write a snapshot of given registry to the
  // array
  this->snapshots_[this->next_snapshot_to_write_] = in_registry.clone();
  // Increment and return
  this->next_snapshot_to_write_++;
  return true;
}

bool RegPack::ReadSnapshot(entt::registry& in_registry) {
  // If end reached return false
  if (this->next_snapshot_to_read_ >= this->num_of_snapshots_) {
    return false;
  }
  // Read a snapshot into the given registry from the
  // array
  in_registry = this->snapshots_[this->next_snapshot_to_read_].clone();
  // Increment and return
  this->next_snapshot_to_read_++;
  return true;
}
