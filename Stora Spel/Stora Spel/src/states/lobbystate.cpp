#include "state.hpp"

#include "engine.hpp"
#include "entitycreation.hpp"

void LobbyState::Startup() {
  auto font_test_ = glob::GetFont("assets/fonts/fonts/ariblk.ttf");
  ButtonComponent* button_c = GenerateButtonEntity(registry_lobby_, "READY",
                                              glm::vec2(100, 200), font_test_);
  button_c->button_func = [&]() { 
    auto& packet = engine_->GetPacket();
    packet << PacketBlockType::CLIENT_READY;
  }; 
}

void LobbyState::Init() {
  //
  engine_->SetSendInput(false);
  engine_->SetCurrentRegistry(&registry_lobby_);

  engine_->GetClient().Connect("localhost", 1337);

  engine_->SetEnableChat(true);
}

void LobbyState::Update() {
  //
}

void LobbyState::UpdateNetwork() {

}

void LobbyState::Cleanup() {
  //
}
