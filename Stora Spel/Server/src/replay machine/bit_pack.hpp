#ifndef BIT_PACK_HPP_
#define BIT_PACK_HPP_

#include <string>

class BitPack {
 private:
  int* bits_;

  unsigned int num_of_bits_;
  
  unsigned int next_bit_to_write_;
  unsigned int next_bit_to_read_;

  unsigned int end_bit_;

 public:
  BitPack(unsigned int in_num_of_frames, unsigned int in_bits_per_frame);
  ~BitPack();

  void ResetWrite();
  void ResetRead();
  bool IsWriteAtEnd();
  bool IsReadAtEnd();
  unsigned int GetNextWrittenBitIndex() const;
  unsigned int GetNextReadBitIndex() const;
  //---
  bool WriteBit(bool in_bit);
  bool WriteInt(unsigned int in_int, unsigned int in_bit_count);
  bool WriteFloat32(float in_float);
  bool ReadBit();
  unsigned int ReadInt(unsigned int in_bit_count);
  float ReadFloat32();
  //---
  bool SaveToFile(std::string in_path);
  bool LoadFromFile(std::string in_path);

};

#endif  // !BIT_PACK_HPP_
