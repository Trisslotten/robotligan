#include <glob/window.hpp>
#include <GLFW/glfw3.h>
#include <glob/window.hpp>
#include "engine.hpp"
#include "entitycreation.hpp"
#include "state.hpp"
#include "util/input.hpp"
void ConnectMenuState::Startup() {
  auto& client = engine_->GetClient();
  glob::window::SetMouseLocked(false);
  font_test_ = glob::GetFont("assets/fonts/fonts/ariblk.ttf");

  // PLAY BUTTON - change registry to registry_gameplay_
  ButtonComponent* b_c = GenerateButtonEntity(
      registry_connect_menu_, "Connect",
      glm::vec2((glob::window::GetWindowDimensions().x / 2.0f) -
                    glob::window::GetWindowDimensions().x * 0.1,
                (glob::window::GetWindowDimensions()).y / 2.0f),
      font_test_);
  b_c->button_func = [&]() {
    if ((ip_.length() > 0) && (port_.length() > 0)) {
      client.Disconnect();
      connection_success_ =
          client.Connect(ip_.c_str(), (short)std::stoi(port_));
      isconnected_ = 1;
    }
  };
  glm::vec2 ip_pos =
      glm::vec2((glob::window::GetWindowDimensions().x / 2.0f) -
                    glob::window::GetWindowDimensions().x * 0.05,
                (glob::window::GetWindowDimensions().y / 2.5f) - 10.f);
  glm::vec2 port_pos =
      glm::vec2((glob::window::GetWindowDimensions().x / 2.0f) -
                    glob::window::GetWindowDimensions().x * 0.05,
                (glob::window::GetWindowDimensions().y / 2.8f) - 20.f);

  auto ip = registry_connect_menu_.create();
  auto& ip_field = registry_connect_menu_.assign<InputComponent>(ip);
  ip_field.pos = ip_pos;
  ip_field.input_name = "IP";
  ip_field.font_size = 32;
  ip_field.text = ip_;
  ip_field.linked_value = &ip_;

  auto port = registry_connect_menu_.create();
  auto& port_field = registry_connect_menu_.assign<InputComponent>(port);
  port_field.pos = port_pos;
  port_field.input_name = "PORT";
  port_field.font_size = 32;
  port_field.text = port_;
  port_field.linked_value = &port_;
}

void ConnectMenuState::Init() {
  auto& client = engine_->GetClient();
  //
  engine_->SetSendInput(false);
  engine_->SetCurrentRegistry(&registry_connect_menu_);
  engine_->SetEnableChat(false);
  last_msg_ = "Not Connected";
  client.Disconnect();
  isconnected_ = 0;
}

void ConnectMenuState::Update(float dt) {
  auto& client = engine_->GetClient();
  auto isDown = glob::window::MouseButtonDown(GLFW_MOUSE_BUTTON_1);
  auto windowSize = glob::window::GetWindowDimensions();

  isconnected_ = engine_->IsConnected();
  auto pos = glm::vec2((glob::window::GetWindowDimensions().x / 2.0f),
                       glob::window::GetWindowDimensions().y / 5.f);
  if (connection_success_) {
    if (isconnected_ == 0) {
      glob::Submit(font_test_, pos, 45, std::string("Not Connected"),
                   glm::vec4(1, 1, 1, 1));
      frames_ = 0;
    } else if (isconnected_ == 1 || frames_ < 2 * kClientUpdateRate) {
      glob::Submit(font_test_, pos, 45, std::string("Connecting..."),
                   glm::vec4(1, 1, 1, 1));
      last_msg_ = "Connecting...";
      frames_++;
    } else if (isconnected_ == 1 &&
               frames_ > kClientUpdateRate * kServerTimeout) {
      glob::Submit(font_test_, pos, 45,
                   std::string("Failed to connect: Timeout"),
                   glm::vec4(1, 1, 1, 1));
      last_msg_ = "Failed to connect: Timeout";
      connection_success_ = false;
      client.Disconnect();
    } else if (isconnected_ == 2) {
      // Connected
      frames_ = 0;
      engine_->ChangeState(StateType::LOBBY);
    } else if (isconnected_ == -2) {
      glob::Submit(font_test_, pos, 45,
                   std::string("Could not connect: Server full"),
                   glm::vec4(1, 1, 1, 1));
      last_msg_ = "Could not connect: Server full";
      connection_success_ = false;
      client.Disconnect();
    }
  } else {
    glob::Submit(font_test_, pos, 45, last_msg_, glm::vec4(1, 1, 1, 1));
  }
}

void ConnectMenuState::UpdateNetwork() {}

void ConnectMenuState::Cleanup() {}
