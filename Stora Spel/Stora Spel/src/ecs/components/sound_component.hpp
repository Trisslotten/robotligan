#ifndef SOUND_COMPONENT_HPP_
#define SOUND_COMPONENT_HPP_

#include <slob/sound_engine.hpp>

struct SoundComponent {
  slob::SoundPlayer* sound_player = nullptr;
};

#endif // SOUND_COMPONENT_HPP_