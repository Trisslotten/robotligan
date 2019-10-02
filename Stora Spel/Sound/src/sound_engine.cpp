#include "sound_engine.hpp"

#include <iostream>

#include <string>
#include <unordered_map>
#include <vector>

#include <fmod_api_core_inc/fmod_errors.h>
#include <fmod_api_core_inc/fmod.hpp>
#include <glm/ext.hpp>

namespace slob {

class EXPORT Sound {
 public:
  void LoadFromFile(const std::string& path, FMOD::System* system) {
    system->createSound(path.c_str(), FMOD_3D, nullptr, &sound_);
    sound_->setMode(FMOD_LOOP_NORMAL);
  }

  void Play(FMOD::System* system, FMOD::ChannelGroup* group, int loop_count) {
    FMOD::Channel* channel = nullptr;
    FMOD_RESULT result = system->playSound(sound_, group, true, &channel);
    channel->setLoopCount(loop_count);
    channel->setVolume(volume_);
    channel->setPaused(false);
  }

  void SetVolume(float vol) { volume_ = vol; }

  void Set3DAttributes(glm::vec3 pos, glm::vec3 vel) {
    pos_ = pos;
    vel_ = vel;
  }

  bool IsLoaded() { return sound_ != nullptr; }

 private:
  FMOD::Sound* sound_ = nullptr;
  float volume_ = 1.0f;
  glm::vec3 pos_{1.0f};
  glm::vec3 vel_{0.0f};
};

struct SoundEngine::Impl {
  std::unordered_map<std::string, SoundHandle> sound_handles;
  std::unordered_map<SoundHandle, Sound> sounds;
  FMOD::System* system = nullptr;
  FMOD::ChannelGroup* channel_group = nullptr;
  std::vector<SoundPlayer*> sound_players;
  float master_volume = 1.0f;

  SoundHandle sound_handle_guid = 0;
};

// H=Handle, A=Asset
template <class H, class A>
H GetAsset(std::unordered_map<std::string, H>& handles,
           std::unordered_map<H, A>& assets, H& guid,
           const std::string filepath, FMOD::System* system) {
  H result = 0;

  auto item = handles.find(filepath);
  if (item == handles.end()) {
    std::cout << "DEBUG sound_engine.cpp: Loading asset '" << filepath << "'\n";
    A& asset = assets[guid];
    asset.LoadFromFile(filepath, system);
    if (asset.IsLoaded()) {
      handles[filepath] = guid;
      result = guid;
      guid++;
    } else {
      // remove the asset since it could not load
      assets.erase(guid);
    }
  } else {
    // if asset is loaded
    std::cout << "DEBUG sound_engine.cpp: Asset '" << filepath
              << "' already loaded\n";
    result = item->second;
  }

  return result;
}

SoundEngine::SoundEngine() { this->impl_ = new Impl(); }

SoundEngine::~SoundEngine() {
  delete impl_;
  for (auto sp : impl_->sound_players) {
    delete sp;
  }
}

void SoundEngine::Init() {
  FMOD_RESULT result;

  result =
      FMOD::System_Create(&impl_->system);  // Create the main system object.
  if (result != FMOD_OK) {
    printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
    exit(-1);
  }

  result = impl_->system->init(512, FMOD_INIT_NORMAL, 0);  // Initialize FMOD.
  if (result != FMOD_OK) {
    printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
    exit(-1);
  }
  FMOD_VECTOR listener_pos{0.0f, 0.0f, 0.0f};
  FMOD_VECTOR listener_forward{1.0f, 0.0f, 0.0f};
  FMOD_VECTOR listener_up{0.0f, 1.0f, 0.0f};
  FMOD_VECTOR listener_vel{0.0f, 0.0f, 0.0f};

  impl_->system->set3DSettings(1.f, 1.0f, 1.0f);
  impl_->system->set3DListenerAttributes(0, &listener_pos, &listener_vel,
                                          &listener_forward, &listener_up);

}

void SoundEngine::Update() { impl_->system->update(); }

SoundHandle SoundEngine::GetSound(const std::string& path) {
  return GetAsset(impl_->sound_handles, impl_->sounds,
                  impl_->sound_handle_guid, path, impl_->system);
}

void SoundEngine::SetMasterVolume(float vol) { impl_->master_volume = vol;
  FMOD::ChannelGroup* master_group = nullptr;
  impl_->system->getMasterChannelGroup(&master_group);
  master_group->setVolume(vol);
}

void SoundEngine::SetListenerAttributes(glm::vec3 pos, glm::quat orientation,
                                        glm::vec3 vel) {
  glm::vec3 forward = orientation * glm::vec3(1, 0, 0);
  glm::vec3 up = orientation * glm::vec3(0, 1, 0);
  impl_->system->set3DListenerAttributes(
      0, (FMOD_VECTOR*)&pos, (FMOD_VECTOR*)&vel, (FMOD_VECTOR*)&forward,
      (FMOD_VECTOR*)&up);
}

SoundPlayer* SoundEngine::CreatePlayer() {
  auto result = new SoundPlayer(impl_);
  impl_->sound_players.push_back(result);
  return result;
}

struct SoundPlayer::Impl {
  SoundEngine::Impl* sound_engine;
  FMOD::ChannelGroup* group = nullptr;
};

SoundPlayer::SoundPlayer(void* engine_impl) {
  i_ = new Impl();
  i_->sound_engine = (SoundEngine::Impl*)engine_impl;
  i_->sound_engine->system->createChannelGroup(nullptr, &i_->group);
  i_->group->setMode(FMOD_3D);
}
SoundPlayer::~SoundPlayer() { delete i_; }

void SoundPlayer::Play(SoundHandle handle, int loop_count) {
  auto iter = i_->sound_engine->sounds.find(handle);
  if (iter != i_->sound_engine->sounds.end()) {
    iter->second.Play(i_->sound_engine->system, i_->group, loop_count);
  }
}

void SoundPlayer::Set3DAttributes(glm::vec3 pos, glm::vec3 vel) {
  auto result =
      i_->group->set3DAttributes((FMOD_VECTOR*)&pos, (FMOD_VECTOR*)&vel);
}

}  // namespace slob