#include "sound_engine.hpp"

#include <iostream>

#include <string>
#include <unordered_map>
#include <vector>

#include <fmod_api_core_inc/fmod_errors.h>
#include <fmod_api_core_inc/fmod.hpp>

namespace slob {

class EXPORT Sound {
 public:

  void LoadFromFile(const std::string& path, FMOD::System* system) {
    system->createSound(path.c_str(), FMOD_LOOP_OFF, nullptr, &sound_);
  }

  void Play(FMOD::System* system, FMOD::ChannelGroup* group) {
    FMOD_RESULT result = system->playSound(sound_, group, false, &channel_);
    std::cout << result << "\n";
    channel_->setChannelGroup(group);
  }

  bool IsLoaded() { return sound_ != nullptr; }

 private:
  FMOD::Sound* sound_ = nullptr;
  FMOD::Channel* channel_ = nullptr;
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

struct SoundEngine::Impl {
  std::unordered_map<std::string, SoundHandle> sound_handles_;
  std::unordered_map<SoundHandle, Sound> sounds_;
  FMOD::System* system_ = nullptr;
  FMOD::ChannelGroup* channel_group_ = nullptr;

  SoundHandle sound_handle_guid_ = 0;
};

SoundEngine::SoundEngine() { this->impl_ = new Impl(); }

SoundEngine::~SoundEngine() { delete impl_; }

void SoundEngine::Init() {
  FMOD_RESULT result;

  result =
      FMOD::System_Create(&impl_->system_);  // Create the main system object.
  if (result != FMOD_OK) {
    printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
    exit(-1);
  }

  result = impl_->system_->init(512, FMOD_INIT_NORMAL, 0);  // Initialize FMOD.
  if (result != FMOD_OK) {
    printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
    exit(-1);
  }
}

SoundHandle SoundEngine::GetSound(const std::string& path) {
  return GetAsset(impl_->sound_handles_, impl_->sounds_,
                                impl_->sound_handle_guid_, path, impl_->system_);
}

void SoundEngine::PlaySound(SoundHandle handle) {
  // Assign channel to channel group
  auto iter = impl_->sounds_.find(handle);
  // if found
  if (iter != impl_->sounds_.end()) {
    auto& sound = iter->second;

	sound.Play(impl_->system_, impl_->channel_group_);    
  }
}
}  // namespace slob