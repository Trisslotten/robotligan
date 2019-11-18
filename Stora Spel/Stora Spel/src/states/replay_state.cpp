#include "state.hpp"

#include <glob/graphics.hpp>
#include <glob/window.hpp>
#include "engine.hpp"

// Replay Stuff --------------------------------
// Private:

void ReplayState::Startup() {
  font_test_ = glob::GetFont("assets/fonts/fonts/ariblk.ttf");
}

// Public:
void ReplayState::Init() {
  // Set this to be the current registry of engine
  this->engine_->SetCurrentRegistry(&this->replay_registry_);

  // WIP: Fix with other stuff here
}

void ReplayState::Update(float dt) {
  // Highlight loop logic




  //-------Draw scoreboard during highlight time--------------
  engine_->DrawScoreboard();

  glm::vec2 pos = glob::window::GetWindowDimensions();
  pos /= 2;
  pos.y -= 160;

  int game_end_timeout = engine_->GetReplayMachinePtr()->ReplayLength();

  std::string end_countdown_text =
      std::to_string((int)(game_end_timeout - end_game_timer_.Elapsed()));

  std::string return_to_lobby_test =
      "Returning to lobby in: " + end_countdown_text;

  double width = glob::GetWidthOfText(font_test_, return_to_lobby_test, 48);
  pos.x = (glob::window::GetWindowDimensions().x / 2) - (width / 2);

  glob::Submit(font_test_, pos + glm::vec2(0, -40), 48, return_to_lobby_test);

  std::string best_team = "BLUE";
  glm::vec4 best_team_color = glm::vec4(0.13f, 0.13f, 1.f, 1.f);

  if (engine_->GetTeamScores()[0] > engine_->GetTeamScores()[1]) {
    best_team = "RED";
    best_team_color = glm::vec4(1.f, 0.13f, 0.13f, 1.f);
  }

  std::string winnin_team_text = best_team + " wins!";
  width = glob::GetWidthOfText(font_test_, winnin_team_text, 48);

  pos.x -= width / 2;

  glob::Submit(font_test_, pos + glm::vec2(1, -1), 48, winnin_team_text,
               glm::vec4(0, 0, 0, 0.7f));

  glob::Submit(font_test_, pos, 48, winnin_team_text, best_team_color);

  //-------Draw scoreboard during highlight time--------------
}

void ReplayState::StartReplayMode() {
  if (replaying_) {
    return;
  }

  // Clear replay register
  this->replay_registry_.reset();

  // WIP: WORKING HERE
  /*replaying_ = true;
  replay_counter_ = replay_machine_->NumberOfStoredReplays() - 1;
  replay_machine_->SelectReplay(replay_counter_);*/
}

void ReplayState::PlayReplay() {
  if (!replaying_) {
    return;
  }

  ClientReplayMachine* engine_rpm = this->engine_->GetReplayMachinePtr();

  if (engine_rpm->LoadFrame(this->replay_registry_)) {
    // Once replay is done playing, clear the registry
    this->replay_registry_.reset();

    // And stop replaying
    this->replaying_ = false;
  }
}

// Replay Stuff --------------------------------