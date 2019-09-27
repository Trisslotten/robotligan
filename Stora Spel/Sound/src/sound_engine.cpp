#include "sound_engine.hpp"

#include <iostream>

namespace slob {

SoundEngine::SoundEngine() {}

SoundEngine::~SoundEngine() {}

void SoundEngine::Init() {
  FMOD_RESULT result;

  result = FMOD::System_Create(&system_);  // Create the main system object.
  if (result != FMOD_OK) {
    printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
    exit(-1);
  }

  result = system_->init(512, FMOD_INIT_NORMAL, 0);  // Initialize FMOD.
  if (result != FMOD_OK) {
    printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
    exit(-1);
  }
}
}  // namespace slob

//#include "../include/slob/sound.hpp"
//
//#include <fmod_api_core_inc/fmod.hpp>
////#include <fmod_api_studio_inc/fmod_studio.h>
//
// namespace slob {
// namespace {
//
//
//
//}  // namespace
//
//
//}  // namespace slob