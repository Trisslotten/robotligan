
#include <glob/window.hpp>
#include <GLFW/glfw3.h>
#include "engine.hpp"
#include "entitycreation.hpp"
#include "state.hpp"
#include "util/input.hpp"
#include "../ecs/systems/input_system.hpp"
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
    if (!ip_field_.input_field.empty() && !port_field_.input_field.empty() &&
        !client.IsConnected()) {
      client.Connect(ip_field_.input_field.c_str(),
                     std::stoi(port_field_.input_field));
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
                (glob::window::GetWindowDimensions().y / 2.8f) - 10.f);
  ip_field_.pos = ip_pos;
  ip_field_.size = glm::vec2(50.0f, 10.0f);
  ip_field_.hndl = glob::GetGUIItem("Assets/GUI_elements/input_field.png");
  ip_field_.input_field = "localhost";
 

  port_field_.pos = port_pos;
  port_field_.size = glm::vec2(50.0f, 10.0f);
  port_field_.hndl = glob::GetGUIItem("Assets/GUI_elements/input_field.png");
  port_field_.input_field = "1337";
}

void ConnectMenuState::Init() {
  auto& client = engine_->GetClient();
  //
  engine_->SetSendInput(false);
  engine_->SetCurrentRegistry(&registry_connect_menu_);
  engine_->SetEnableChat(false);
}

void ConnectMenuState::Update(float dt) {
  auto& client = engine_->GetClient();
  auto isDown = glob::window::MouseButtonDown(GLFW_MOUSE_BUTTON_1);
  auto windowSize = glob::window::GetWindowDimensions();
  if (isDown) {
    auto mousepos = glob::window::MousePosition();
    if ((mousepos.x > ip_field_.pos.x && mousepos.x < ip_field_.pos.x + 100) &&
        (mousepos.y > windowSize.y - ip_field_.pos.y - 10 &&
         mousepos.y < windowSize.y - ip_field_.pos.y)) {
      ip_field_.focus = true;
      port_field_.focus = false;
    } else if ((mousepos.x > port_field_.pos.x &&
                mousepos.x < port_field_.pos.x + 100) &&
               (mousepos.y > windowSize.y - port_field_.pos.y - 10 &&
                mousepos.y < windowSize.y - port_field_.pos.y)) {
      port_field_.focus = true;
      ip_field_.focus = false;
    } else {
      port_field_.focus = false;
      ip_field_.focus = false;
    }
  }
  if (ip_field_.focus) {
    if (ip_field_.input_field.length() < 15) {
      ip_field_.input_field += Input::GetCharacters();
    }
    if (Input::IsKeyPressed(GLFW_KEY_BACKSPACE) &&
        ip_field_.input_field.size() > 0) {
      ip_field_.input_field.pop_back();
    }
  } else if (port_field_.focus) {
    if (port_field_.input_field.length() < 5) {
      port_field_.input_field += Input::GetCharacters();
    }
    if (Input::IsKeyPressed(GLFW_KEY_BACKSPACE) &&
        port_field_.input_field.size() > 0) {
      port_field_.input_field.pop_back();
    }
  }
  auto sub_posy = ip_field_.pos.y + 10;
  glob::Submit(font_test_,
               glm::vec2((glob::window::GetWindowDimensions().x / 2.0f) -
                             glob::window::GetWindowDimensions().x * 0.1,
                         glob::window::GetWindowDimensions().y / 2.5f),
               25, std::string("IP:"), glm::vec4(1, 1, 1, 1));
  glob::Submit(font_test_,
               glm::vec2((glob::window::GetWindowDimensions().x / 2.0f) -
                             glob::window::GetWindowDimensions().x * 0.1,
                         glob::window::GetWindowDimensions().y / 2.8f),
               25, std::string("PORT:"), glm::vec4(1, 1, 1, 1));
  glob::Submit(font_test_, glm::vec2(ip_field_.pos.x + 10, sub_posy), 20,
               ip_field_.input_field, glm::vec4(0.0f, 0.0f, 0.0f, 1));
  sub_posy = port_field_.pos.y + 10;
  glob::Submit(font_test_, glm::vec2(port_field_.pos.x + 10, sub_posy), 20,
               port_field_.input_field, glm::vec4(0.0f, 0.0f, 0.0f, 1));
  glob::Submit(ip_field_.hndl, ip_field_.pos, 0.5f, 150.0f);
  glob::Submit(port_field_.hndl, port_field_.pos, 0.5f, 150.0f);

  glob::Submit(font_test_,
               glm::vec2((glob::window::GetWindowDimensions().x / 2.0f) -
                             glob::window::GetWindowDimensions().x * 0.1,
                         glob::window::GetWindowDimensions().y / 2.8f),
               25, std::string("PORT:"), glm::vec4(1, 1, 1, 1));
      auto isconnected_ = engine_->IsConnected();
	  if (isconnected_ == 0)
	  {
		 auto pos = glm::vec2((glob::window::GetWindowDimensions().x / 2.0f) -
			  glob::window::GetWindowDimensions().x,
			 (glob::window::GetWindowDimensions()).y / 2.0f);
		 glob::Submit(font_test_,
			 glm::vec2((glob::window::GetWindowDimensions().x / 2.0f),
				 glob::window::GetWindowDimensions().y / 5.f),
			 45, std::string("Not Connected"), glm::vec4(1, 1, 1, 1));
	  }
	  else if (isconnected_ == 1)
	  {
		  //Connecting
	  }
	  else if (isconnected_ == 2)
	  {
		  //Connected
		  engine_->ChangeState(StateType::LOBBY);
	  }
	  else if (isconnected_ == -1)
	  {
		  
	  }
	  else if (isconnected_ == -2)
	  {
		  auto pos = glm::vec2((glob::window::GetWindowDimensions().x / 2.0f) -
			  glob::window::GetWindowDimensions().x,
			  (glob::window::GetWindowDimensions()).y / 2.0f);
		  glob::Submit(font_test_,
			  glm::vec2((glob::window::GetWindowDimensions().x / 2.0f),
				  glob::window::GetWindowDimensions().y / 5.f),
			  45, std::string("Not Connected: Server full"), glm::vec4(1, 1, 1, 1));
		  client.Disconnect();
	  }
}

void ConnectMenuState::UpdateNetwork() {}

void ConnectMenuState::Cleanup() {}
