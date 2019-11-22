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
  auto dim = glob::window::GetWindowDimensions();
  // PLAY BUTTON - change registry to registry_gameplay_
  ButtonComponent* b_c = GenerateButtonEntity(
      registry_connect_menu_, "CONNECT",
      glm::vec2(dim.x / 2.0f - 90, dim.y / 2.0f - 110), font_test_);
  b_c->button_func = [&]() {
    if ((ip_.length() > 0) && (port_.length() > 0)) {
      client.Disconnect();
      connection_success_ =
          client.Connect(ip_.c_str(), (short)std::stoi(port_));
      isconnected_ = 1;
    }
  };
  ButtonComponent* b_back = GenerateButtonEntity(registry_connect_menu_, "BACK",
                                                 glm::vec2(60, 50), font_test_);
  b_back->button_func = [&]() { engine_->ChangeState(StateType::MAIN_MENU); };

  glm::vec2 ip_pos(dim.x / 2.0f - 191 / 2.f, dim.y / 2.0f - 27 / 2.f + 35);
  glm::vec2 port_pos(dim.x / 2.0f - 191 / 2.f, dim.y / 2.0f - 27 / 2.f - 35);

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
  bg_ = glob::GetGUIItem("Assets/GUI_elements/connect-23.png");
  CreateBackground();
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
  auto windowsize = glob::window::GetWindowDimensions();
  auto is_enter = glob::window::KeyDown(GLFW_KEY_ENTER);
  auto is_escape = glob::window::KeyDown(GLFW_KEY_ESCAPE);
  isconnected_ = engine_->IsConnected();

  double text_width = glob::GetWidthOfText(font_test_, "Server Info", 45);
  auto gamepos = glm::vec2((windowsize.x / 2.f - text_width / 2.f),
                           windowsize.y * 0.5f + 130);
  auto bgpos = glm::vec2((windowsize.x / 2.f - 421.f / 2.f),
                         windowsize.y / 2.f - 408.f / 2.f);
  glob::Submit(font_test_, gamepos, 45, "Server Info", glm::vec4(1, 1, 1, 1));
  glob::Submit(bg_, bgpos, 1.0);
  if (is_enter) {
    MenuEvent click_event;
    click_event.type = MenuEvent::CLICK;
    menu_dispatcher.trigger(click_event);
    if ((ip_.length() > 0) && (port_.length() > 0)) {
      client.Disconnect();
      connection_success_ =
          client.Connect(ip_.c_str(), (short)std::stoi(port_));
      isconnected_ = 1;
    }
  } else if (is_escape) {
    MenuEvent click_event;
    click_event.type = MenuEvent::CLICK;
    menu_dispatcher.trigger(click_event);
    client.Disconnect();
    engine_->ChangeState(StateType::MAIN_MENU);
  }
  std::string status = "STATUS: ";
  auto pos =
      glm::vec2((windowsize.x - windowsize.x * 0.25), windowsize.y * 0.03);
  if (connection_success_) {
    if (isconnected_ == 0) {
      glob::Submit(font_test_, pos, 45, status + std::string("Not Connected"),
                   glm::vec4(1, 1, 1, 1));
      frames_ = 0;
    } else if (isconnected_ == 1 || frames_ < 2 * kClientUpdateRate) {
      glob::Submit(font_test_, pos, 45, status + std::string("Connecting..."),
                   glm::vec4(1, 1, 1, 1));
      last_msg_ = status + "Connecting...";
      frames_++;
    } else if (isconnected_ == 1 &&
               frames_ > kClientUpdateRate * kServerTimeout) {
      glob::Submit(font_test_, pos, 45,
                   status + std::string("Failed to connect, Timeout"),
                   glm::vec4(1, 1, 1, 1));
      last_msg_ = status + "Failed to connect, Timeout";
      connection_success_ = false;
      client.Disconnect();
    } else if (isconnected_ == 2) {
      // Connected
      frames_ = 0;
      engine_->ChangeState(StateType::LOBBY);
    } else if (isconnected_ == -2) {
      glob::Submit(font_test_, pos, 45,
                   status + std::string("Could not connect, Server full"),
                   glm::vec4(1, 1, 1, 1));
      last_msg_ = status + "Could not connect, Server full";
      connection_success_ = false;
      client.Disconnect();
    } else if (isconnected_ == -3) {
      glob::Submit(font_test_, pos, 45,
                   status + std::string("Could not connect, No Ident"),
                   glm::vec4(1, 1, 1, 1));
      last_msg_ = status + "Could not connect, No Ident";
      connection_success_ = false;
      client.Disconnect();
    }
  } else {
    glob::Submit(font_test_, pos, 45, last_msg_, glm::vec4(1, 1, 1, 1));
  }
}
void ConnectMenuState::CreateBackground() {
  auto light_test = registry_connect_menu_.create();  // Get from engine
  registry_connect_menu_.assign<LightComponent>(light_test, glm::vec3(0.05f),
                                                30.f, 0.2f);
  registry_connect_menu_.assign<TransformComponent>(
      light_test, glm::vec3(0.f, 16.f, 0.f), glm::vec3(0.f, 0.f, 1.f),
      glm::vec3(1.f));
  glm::vec3 zero_vec = glm::vec3(0.0f);

  auto light_test2 = registry_connect_menu_.create();  // Get from engine
  registry_connect_menu_.assign<LightComponent>(
      light_test2, glm::vec3(0.f, 0.f, 1.0f), 50.f, 0.2f);
  registry_connect_menu_.assign<TransformComponent>(
      light_test2, glm::vec3(48.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 1.f),
      glm::vec3(1.f));
  glm::vec3 arena_scale = glm::vec3(2.0f);
  {
    // ladda in och skapa entity för bana
    auto arena = registry_connect_menu_.create();
    glm::vec3 zero_vec = glm::vec3(0.0f);
    glob::ModelHandle model_arena =
        glob::GetModel("assets/Arena/Map_V3_ARENA.fbx");
    glob::ModelHandle model_arena_banner =
        glob::GetModel("assets/Arena/Map_V3_ARENA_SIGNS.fbx");
    glob::ModelHandle model_map = glob::GetModel("assets/MapV3/Map_Walls.fbx");
    glob::ModelHandle model_map_floor =
        glob::GetModel("assets/MapV3/Map_Floor.fbx");
    glob::ModelHandle model_map_projectors =
        glob::GetModel("assets/MapV3/Map_Projectors.fbx");

    // glob::GetModel(kModelPathMapSingular);
    auto& model_c = registry_connect_menu_.assign<ModelComponent>(arena);
    model_c.handles.push_back(model_arena);
    model_c.handles.push_back(model_arena_banner);
    model_c.handles.push_back(model_map_projectors);

    registry_connect_menu_.assign<TransformComponent>(arena, zero_vec, zero_vec,
                                                      arena_scale);

    arena = registry_connect_menu_.create();
    auto& model_c2 = registry_connect_menu_.assign<ModelComponent>(arena);
    model_c2.handles.push_back(model_map);
    model_c2.handles.push_back(model_map_floor);
    registry_connect_menu_.assign<TransformComponent>(arena, zero_vec, zero_vec,
                                                      arena_scale);
  }
  {
    auto arena = registry_connect_menu_.create();
    glob::ModelHandle model_map_walls =
        glob::GetTransparentModel("assets/MapV3/Map_EnergyWall.fbx");

    auto& model_c = registry_connect_menu_.assign<ModelComponent>(arena);
    model_c.handles.push_back(model_map_walls);
    registry_connect_menu_.assign<TransformComponent>(arena, zero_vec, zero_vec,
                                                      arena_scale);
  }
  {
    auto robot = registry_connect_menu_.create();
    auto& trans_c = registry_connect_menu_.assign<TransformComponent>(
        robot, glm::vec3(30.f, 0.2, -2.1f),
        glm::vec3(0.f, glm::radians(-135.0f), 0.f), glm::vec3(0.0033f));
    glob::ModelHandle model_robot = glob::GetModel("assets/Mech/Mech.fbx");
    auto& model_c = registry_connect_menu_.assign<ModelComponent>(robot);
    model_c.handles.push_back(model_robot);

    // Animation
    auto& animation_c = registry_connect_menu_.assign<AnimationComponent>(
        robot, glob::GetAnimationData(model_robot));

    engine_->GetAnimationSystem().PlayAnimation(
        "Resting", 0.7f, &animation_c, 10, 1.f,
        engine_->GetAnimationSystem().LOOP);
  }
  {
    auto robot = registry_connect_menu_.create();
    auto& trans_c = registry_connect_menu_.assign<TransformComponent>(
        robot, glm::vec3(30.f, 0.2, 2.1f),
        glm::vec3(0.f, glm::radians(135.0f), 0.f), glm::vec3(0.0033f));
    glob::ModelHandle model_robot = glob::GetModel("assets/Mech/Mech.fbx");

    auto& model_c = registry_connect_menu_.assign<ModelComponent>(robot);
    model_c.diffuse_index = 1;
    model_c.handles.push_back(model_robot);

    // Animation
    auto& animation_c = registry_connect_menu_.assign<AnimationComponent>(
        robot, glob::GetAnimationData(model_robot));

    engine_->GetAnimationSystem().PlayAnimation(
        "Resting", 0.7f, &animation_c, 10, 1.f,
        engine_->GetAnimationSystem().LOOP);
  }

  auto camera = registry_connect_menu_.create();
  auto& cam_c = registry_connect_menu_.assign<CameraComponent>(camera);
  auto& cam_trans = registry_connect_menu_.assign<TransformComponent>(camera);
  cam_trans.position = glm::vec3(28.f, 1.5f, 0.f);
  glm::vec3 dir = glm::vec3(0) - cam_trans.position;
  cam_c.orientation = glm::quat(glm::vec3(0.f, 0.f, 0.f));
}
void ConnectMenuState::UpdateNetwork() {}

void ConnectMenuState::Cleanup() {}
