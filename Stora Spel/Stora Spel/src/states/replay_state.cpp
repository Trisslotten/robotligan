#include "state.hpp"

#include <glob/graphics.hpp>
#include <glob/window.hpp>
#include "engine.hpp"

// Replay Stuff --------------------------------
// Private:

// Public:
void ReplayState::Startup() {
  font_test_ = glob::GetFont("assets/fonts/fonts/ariblk.ttf");
}


void ReplayState::Init() {
  // Set this to be the current registry of engine
  this->engine_->SetCurrentRegistry(&this->replay_registry_);

  StartReplayMode();
}

void ReplayState::Update(float dt) {
  // Highlight loop logic
  PlayReplay();
  //-------Draw scoreboard during highlight time--------------
  engine_->DrawScoreboard();

  glm::vec2 pos = glob::window::GetWindowDimensions();
  pos /= 2;
  pos.y -= 160;

  int game_end_timeout = engine_->GetReplayMachinePtr()->ReplayLength() + 5;

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

  if (end_game_timer_.Elapsed() > game_end_timeout) {
    engine_->ChangeState(StateType::LOBBY);
  }
}

void ReplayState::UpdateNetwork() {}

void ReplayState::Cleanup() {
  // Clear registry
  this->replay_registry_.reset();

  // Reset all variables
  this->replaying_ = false;
  this->num_of_replays_ = 0;
  this->replay_counter_ = 0;

  end_game_timer_.Pause();
}

void ReplayState::StartReplayMode() {
  // Do not start if already replaying
  if (replaying_) {
    return;
  }
  // Set replaying to true
  this->replaying_ = true;

  // Get number of replays
  ClientReplayMachine* engine_rpm = this->engine_->GetReplayMachinePtr();
  this->num_of_replays_ = engine_rpm->NumberOfStoredReplays();
  this->replay_counter_ = num_of_replays_ - 1; // Cheat row
  engine_rpm->SelectReplay(this->replay_counter_);
}

void ReplayState::PlayReplay() {
  if (!replaying_) {
    return;
  }

  if (engine_->GetReplayMachinePtr()->LoadFrame(this->replay_registry_)) {
    // Once replay is done playing, clear the registry
    this->replay_registry_.reset();

    // And stop replaying
    this->replaying_ = false;
  }
}

// Replay Stuff --------------------------------