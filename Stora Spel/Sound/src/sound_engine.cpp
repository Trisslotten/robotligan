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
    FMOD_RESULT result = system->createSound(
        path.c_str(), FMOD_CREATECOMPRESSEDSAMPLE, nullptr, &sound_);
    // std::cout << "error nr: " << result << std::endl;
    sound_->setMode(FMOD_LOOP_NORMAL);
  }
  void Play(FMOD::System* system, FMOD::ChannelGroup* group, int loop_count,
            float volume) {
    FMOD::Channel* channel = nullptr;
    FMOD_RESULT result = system->playSound(sound_, group, true, &channel);
    if (result != FMOD_OK) {
      std::cout << "FMOD_ERROR: " << result << "\n";
    }
    channel->setLoopCount(loop_count);
    channel->setVolume(volume);
    channel->setPaused(false);
  }
  void Stop(FMOD::System* system, FMOD::ChannelGroup* group) { group->stop(); }
  bool IsLoaded() { return sound_ != nullptr; }

 private:
  FMOD::Sound* sound_ = nullptr;
};

struct SoundEngine::Impl {
  std::unordered_map<std::string, SoundHandle> sound_handles;
  std::unordered_map<SoundHandle, Sound> sounds;
  FMOD::System* system = nullptr;
  FMOD::ChannelGroup* master_group = nullptr;
  std::vector<SoundPlayer*> sound_players;

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
  for (auto sp : impl_->sound_players) {
    delete sp;
  }
  delete impl_;
}

void SoundEngine::Init() {
  FMOD_RESULT result;

  result =
      FMOD::System_Create(&impl_->system);  // Create the main system object.
  if (result != FMOD_OK) {
    printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
    exit(-1);
  }

  result = impl_->system->init(1024, FMOD_INIT_NORMAL, 0);  // Initialize FMOD.
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
  return GetAsset(impl_->sound_handles, impl_->sounds, impl_->sound_handle_guid,
                  path, impl_->system);
}

void SoundEngine::SetMasterVolume(float vol) {
  impl_->system->getMasterChannelGroup(&impl_->master_group);
  impl_->master_group->setVolume(vol);
}

void SoundEngine::SetListenerAttributes(glm::vec3 pos, glm::quat orientation,
                                        glm::vec3 vel) {
  glm::vec3 forward = orientation * glm::vec3(-1, 0, 0);
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

SoundPlayer* SoundEngine::GetPlayer() { return impl_->sound_players[0]; }

struct SoundPlayer::Impl {
  SoundEngine::Impl* sound_engine;
  FMOD::ChannelGroup* channel_group;
};

SoundPlayer::SoundPlayer(void* engine_impl) {
  i_ = new Impl();
  i_->sound_engine = (SoundEngine::Impl*)engine_impl;
  i_->sound_engine->system->createChannelGroup(nullptr, &i_->channel_group);
  i_->channel_group->setMode(FMOD_3D);
}
SoundPlayer::~SoundPlayer() { delete i_; }

void SoundPlayer::Play(SoundHandle handle, int loop_count, float volume) {
  auto iter = i_->sound_engine->sounds.find(handle);
  if (iter != i_->sound_engine->sounds.end()) {
    iter->second.Play(i_->sound_engine->system, i_->channel_group, loop_count,
                      volume);
  }
}

void SoundPlayer::Stop(SoundHandle handle) {
  auto iter = i_->sound_engine->sounds.find(handle);
  if (iter != i_->sound_engine->sounds.end()) {
    iter->second.Stop(i_->sound_engine->system, i_->channel_group);
  }
}

void SoundPlayer::Set3DAttributes(glm::vec3 pos, glm::vec3 vel) {
  auto result = i_->channel_group->set3DAttributes((FMOD_VECTOR*)&pos,
                                                   (FMOD_VECTOR*)&vel);
}

}  // namespace slob