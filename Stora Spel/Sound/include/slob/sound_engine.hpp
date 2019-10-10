#ifndef SLOB_SOUND_HPP_
#define SLOB_SOUND_HPP_

#ifdef MAKEDLL
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __declspec(dllimport)
#endif

#include <glm/glm.hpp>
#include <string>

namespace slob {
typedef unsigned long SoundHandle;

class EXPORT SoundPlayer {
 public:
  ~SoundPlayer();
  void Play(SoundHandle handle, int loop_count = 0, float volume = 1.0f);
  void Set3DAttributes(glm::vec3 pos, glm::vec3 vel);

 private:
  friend class SoundEngine;
  SoundPlayer(void* engine_impl);
  struct Impl;
  Impl* i_ = nullptr;
};

class EXPORT SoundEngine {
 public:
  SoundEngine();
  ~SoundEngine();
  void Init();
  void Update();
  SoundHandle GetSound(const std::string& path);
  //void SetMasterVolume(float vol);
  void SetListenerAttributes(glm::vec3 pos, glm::quat orientation, glm::vec3 vel);
  SoundPlayer* CreatePlayer();

 private:
  friend class SoundPlayer;
  struct Impl;
  Impl* impl_;
};

}  // namespace slob

#endif  // SLOB_SOUND_HPP_