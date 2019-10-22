#include <glob/graphics.hpp>
#include <glob/window.hpp>
#include <slob/sound_engine.hpp>
#include "util/timer.hpp"

int main() {
  glob::window::Create();
  glob::Init();

  slob::SoundEngine sound_engine;
  sound_engine.Init();

  // Load sounds
  slob::SoundHandle sound_goal = sound_engine.GetSound("assets/sound/goal.mp3");
  slob::SoundHandle sound_step =
      sound_engine.GetSound("assets/sound/footstep.wav");
  sound_engine.SetMasterVolume(2.1f);

  slob::SoundPlayer* sound_player = sound_engine.CreatePlayer();
  //sound_player->Play(sound_goal, -1);

  Timer asd;

  Timer t;
  while (!glob::window::ShouldClose()) {
    float a = asd.Elapsed() * 1.0f;
    glm::vec3 pos = 10.f * glm::vec3(0.1f, 0, sin(a));
    glm::vec3 vel = 10.f * glm::vec3(0, 0, cos(a));
    
    glm::quat orientation{};
    sound_engine.SetListenerAttributes(-pos, orientation, -vel);

    sound_player->Set3DAttributes(pos, vel);
    if (t.Elapsed() > 0.5f) {
      t.Restart();
      sound_player->Play(sound_step, 0);
    }

    sound_engine.Update();
    glob::Render();
    glob::window::Update();
  }

  glob::window::Cleanup();
  return EXIT_SUCCESS;
}