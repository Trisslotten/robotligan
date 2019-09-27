#ifndef SLOB_SOUND_HPP_
#define SLOB_SOUND_HPP_

#ifdef MAKEDLL
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __declspec(dllimport)
#endif

#include <fmod_api_core_inc/fmod.hpp>
#include <fmod_api_core_inc/fmod_errors.h>

namespace slob {

class EXPORT SoundEngine {
 public:
  SoundEngine();
  ~SoundEngine();
  void Init();
  FMOD::System* GetSystem() { return system_; }

 private:
  FMOD::System* system_ = nullptr;
};

}  // namespace slob

#endif  // SLOB_SOUND_HPP_