#ifndef BIT_PACK_HPP_
#define BIT_PACK_HPP_

class BitPack {
 private:
  int* bits_;

  unsigned int arr_size_;
  
  unsigned int next_bit_to_write_;
  unsigned int next_bit_to_read_;

 public:
  BitPack(unsigned int in_num_of_frames, unsigned int in_bits_per_frame);
  ~BitPack();

  void ResetWrite();
  void ResetRead();
  bool WriteBit(bool in_bit);
  bool WriteInteger(unsigned int in_int, unsigned int in_bit_count);
  bool ReadBit() const;
  unsigned int ReadInt(unsigned int in_bit_count) const;
};

#endif  // !BIT_PACK_HPP_
