#ifndef SLOB_SOUND_HPP_
#define SLOB_SOUND_HPP_

#ifdef MAKEDLL
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __declspec(dllimport)
#endif

#include <string>

namespace slob {

typedef unsigned long SoundHandle;

class EXPORT SoundEngine {
 public:
  SoundEngine();
  ~SoundEngine();
  void Init();
  SoundHandle GetSound(const std::string& path);
  void PlaySound(SoundHandle);

 private:
  struct Impl;
  Impl* impl_;
};

}  // namespace slob



#endif  // SLOB_SOUND_HPP_