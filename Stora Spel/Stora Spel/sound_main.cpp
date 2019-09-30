#include <glob/graphics.hpp>
#include <glob/window.hpp>
#include <slob/sound_engine.hpp>
#include "util/timer.hpp"

int main() {
  glob::window::Create();
  glob::Init();

  slob::SoundEngine audio_handler;
  audio_handler.Init();

  // Load sounds
  slob::SoundHandle sound1 = audio_handler.GetSound("assets/sound/crowd.mp3");
  slob::SoundHandle sound2 =
      audio_handler.GetSound("assets/sound/footstep.wav");

  // audio_handler.PlaySound(sound1);

  Timer t;
  while (!glob::window::ShouldClose()) {
    if (t.Elapsed() > 0.1f) {
      t.Restart();
      audio_handler.PlaySound(sound2);
	}
    glob::Render();
    glob::window::Update();
  }

  glob::window::Cleanup();
  return EXIT_SUCCESS;
}