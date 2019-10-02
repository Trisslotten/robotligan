#ifndef REG_PACK_HPP_
#define REG_PACK_HPP_

#include <entity/registry.hpp>

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

#endif  // !REG_PACK_HPP_
