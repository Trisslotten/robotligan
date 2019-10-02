#include "state.hpp"

#include "engine.hpp"
#include "entitycreation.hpp"

void LobbyState::Startup() {
  auto font_test_ = glob::GetFont("assets/fonts/fonts/ariblk.ttf");
  ButtonComponent* button_c = GenerateButtonEntity(registry_lobby_, "READY",
                                              glm::vec2(100, 200), font_test_);
  button_c->button_func = [&]() { 
    //
  }; 
}

void LobbyState::Init() {
  //
  engine_->SetCurrentRegistry(&registry_lobby_);

  engine_->getClient().Connect("localhost", 1337);
}

void LobbyState::Update() {
  //
}

void LobbyState::Cleanup() {
  //
}
