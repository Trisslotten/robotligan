#ifndef REG_PACK_HPP_
#define REG_PACK_HPP_

// #includes for class declaration
#include <entity/registry.hpp>

// #includes for function definitions
// :D

//-----------------------------------------------
//--------------CLASS DECLARATION----------------
//-----------------------------------------------

class RegPack {
 private:
  entt::registry* snapshots_;

  unsigned int num_of_snapshots_;

  unsigned int next_snapshot_to_write_;
  unsigned int next_snapshot_to_read_;

 public:
  RegPack(unsigned int in_num_of_snapshots);
  ~RegPack();

  unsigned int GetNextWrittenSnapshotIndex() const;

  void ResetWrite();
  void ResetRead();

  bool WriteSnapshot(entt::registry& in_registry);
  bool ReadSnapshot(entt::registry& in_registry);

  //bool SaveToFile(std::string in_path);
  //bool LoadFromFile(std::string in_path);
};

//-----------------------------------------------
//------------FUNCTION DEFINITIONS---------------
//-----------------------------------------------

// Private---------------------------------------------------------------------

// Public----------------------------------------------------------------------


inline RegPack::RegPack(unsigned int in_num_of_snapshots) {
  // H
  // A
  // I
  this->snapshots_ = new entt::registry[in_num_of_snapshots];
  this->num_of_snapshots_ = in_num_of_snapshots;

  this->next_snapshot_to_write_ = 0;
  this->next_snapshot_to_read_ = 0;
}

inline RegPack::~RegPack() {
  // B
  // Y
  // E
  delete[] this->snapshots_;
}

inline unsigned int RegPack::GetNextWrittenSnapshotIndex() const {
  // Send back the index of the last snapshot that was written
  return this->next_snapshot_to_write_;
}

inline void RegPack::ResetWrite() {
  // Sets the writing index back to 0.
  this->next_snapshot_to_write_ = 0;
}

inline void RegPack::ResetRead() {
  // Sets the reading index back to 0.
  this->next_snapshot_to_read_ = 0;
}

inline bool RegPack::WriteSnapshot(entt::registry& in_registry) {
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

inline bool RegPack::ReadSnapshot(entt::registry& in_registry) {
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

#endif  // !REG_PACK_HPP_
