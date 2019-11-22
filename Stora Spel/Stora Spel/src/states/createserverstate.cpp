
#include <glob/window.hpp>
#include <GLFW/glfw3.h>
#include "../ecs/components.hpp"
#include "../ecs/systems/animation_system.hpp"
#include "engine.hpp"
#include "entitycreation.hpp"
#include "state.hpp"
#include "../util/winadpihelpers.hpp"
void CreateServerState::Startup() {
  auto windowsize = glob::window::GetWindowDimensions();
  glob::window::SetMouseLocked(false);
  font_test_ = glob::GetFont("assets/fonts/fonts/ariblk.ttf");
  // Buttons
  {
    double text_width = glob::GetWidthOfText(font_test_, "START", 45);
    auto buttonpos = glm::vec2((windowsize.x / 2.f - text_width - 30),
                               windowsize.y * 0.5f - 150);
    ButtonComponent* b_c = GenerateButtonEntity(registry_create_server_, "BACK",
                                                glm::vec2(70, 50), font_test_);
    b_c->button_func = [&]() { engine_->ChangeState(StateType::MAIN_MENU); };
    b_c = GenerateButtonEntity(registry_create_server_, "START", buttonpos,
                               font_test_);
    b_c->button_func = [&]() { CreateServer(); };
  }

  // 3D stuff
  {
    auto light_test = registry_create_server_.create();  // Get from engine
    registry_create_server_.assign<LightComponent>(light_test, glm::vec3(0.05f),
                                                   30.f, 0.2f);
    registry_create_server_.assign<TransformComponent>(
        light_test, glm::vec3(0.f, 16.f, 0.f), glm::vec3(0.f, 0.f, 1.f),
        glm::vec3(1.f));
    glm::vec3 zero_vec = glm::vec3(0.0f);

    auto light_test2 = registry_create_server_.create();  // Get from engine
    registry_create_server_.assign<LightComponent>(
        light_test2, glm::vec3(0.f, 0.f, 1.0f), 50.f, 0.2f);
    registry_create_server_.assign<TransformComponent>(
        light_test2, glm::vec3(48.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 1.f),
        glm::vec3(1.f));
    glm::vec3 arena_scale = glm::vec3(2.0f);
    {
      // ladda in och skapa entity för bana
      auto arena = registry_create_server_.create();
      glm::vec3 zero_vec = glm::vec3(0.0f);
      glob::ModelHandle model_arena =
          glob::GetModel("assets/Arena/Map_V3_ARENA.fbx");
      glob::ModelHandle model_arena_banner =
          glob::GetModel("assets/Arena/Map_V3_ARENA_SIGNS.fbx");
      glob::ModelHandle model_map =
          glob::GetModel("assets/MapV3/Map_Walls.fbx");
      glob::ModelHandle model_map_floor =
          glob::GetModel("assets/MapV3/Map_Floor.fbx");
      glob::ModelHandle model_map_projectors =
          glob::GetModel("assets/MapV3/Map_Projectors.fbx");

      // glob::GetModel(kModelPathMapSingular);
      auto& model_c = registry_create_server_.assign<ModelComponent>(arena);
      model_c.handles.push_back(model_arena);
      model_c.handles.push_back(model_arena_banner);
      model_c.handles.push_back(model_map_projectors);

      registry_create_server_.assign<TransformComponent>(arena, zero_vec,
                                                         zero_vec, arena_scale);

      arena = registry_create_server_.create();
      auto& model_c2 = registry_create_server_.assign<ModelComponent>(arena);
      model_c2.handles.push_back(model_map);
      model_c2.handles.push_back(model_map_floor);
      registry_create_server_.assign<TransformComponent>(arena, zero_vec,
                                                         zero_vec, arena_scale);
    }
    {
      auto arena = registry_create_server_.create();
      glob::ModelHandle model_map_walls =
          glob::GetTransparentModel("assets/MapV3/Map_EnergyWall.fbx");

      auto& model_c = registry_create_server_.assign<ModelComponent>(arena);
      model_c.handles.push_back(model_map_walls);
      registry_create_server_.assign<TransformComponent>(arena, zero_vec,
                                                         zero_vec, arena_scale);
    }
    {
      auto robot = registry_create_server_.create();
      auto& trans_c = registry_create_server_.assign<TransformComponent>(
          robot, glm::vec3(31.f, 0.2, 2.5f),
          glm::vec3(0.f, glm::radians(135.0f), 0.f), glm::vec3(0.0033f));
      glob::ModelHandle model_robot = glob::GetModel("assets/Mech/Mech.fbx");
      auto& model_c = registry_create_server_.assign<ModelComponent>(robot);
      model_c.handles.push_back(model_robot);
      // Animation
      auto& animation_c = registry_create_server_.assign<AnimationComponent>(
          robot, glob::GetAnimationData(model_robot));

      engine_->GetAnimationSystem().PlayAnimation(
          "Kneel", 1.0, &animation_c, 10, 1.f,
          engine_->GetAnimationSystem().LOOP);
    }
    {
      auto robot = registry_create_server_.create();
      auto& trans_c = registry_create_server_.assign<TransformComponent>(
          robot, glm::vec3(32.5f, 0.2, 3.0f),
          glm::vec3(0.f, glm::radians(135.0f), 0.f), glm::vec3(0.0033f));
      glob::ModelHandle model_robot = glob::GetModel("assets/Mech/Mech.fbx");
      auto& model_c = registry_create_server_.assign<ModelComponent>(robot);
      model_c.handles.push_back(model_robot);
      // Animation
      auto& animation_c = registry_create_server_.assign<AnimationComponent>(
          robot, glob::GetAnimationData(model_robot));

      engine_->GetAnimationSystem().PlayAnimation(
          "Emote2", 1.0, &animation_c, 10, 1.f,
          engine_->GetAnimationSystem().LOOP);
    }
    {
      auto robot = registry_create_server_.create();
      auto& trans_c = registry_create_server_.assign<TransformComponent>(
          robot, glm::vec3(30.f, 0.2, 2.1f),
          glm::vec3(0.f, glm::radians(135.0f), 0.f), glm::vec3(0.0033f));
      glob::ModelHandle model_robot = glob::GetModel("assets/Mech/Mech.fbx");

      auto& model_c = registry_create_server_.assign<ModelComponent>(robot);
      model_c.diffuse_index = 1;
      model_c.handles.push_back(model_robot);

      // Animation
      auto& animation_c = registry_create_server_.assign<AnimationComponent>(
          robot, glob::GetAnimationData(model_robot));

      engine_->GetAnimationSystem().PlayAnimation(
          "Kneel", 0.7f, &animation_c, 10, 1.f,
          engine_->GetAnimationSystem().LOOP);
    }

    auto camera = registry_create_server_.create();
    auto& cam_c = registry_create_server_.assign<CameraComponent>(camera);
    auto& cam_trans =
        registry_create_server_.assign<TransformComponent>(camera);
    cam_trans.position = glm::vec3(28.f, 2.0f, 0.f);
    glm::vec3 dir = glm::vec3(0) - cam_trans.position;
    cam_c.orientation = glm::quat(glm::vec3(0.f, 0.f, 0.f));
  }
  // 2D stuff
  bg_ = glob::GetGUIItem("Assets/GUI_elements/connect-23.png");
  // Input fields
  {
    ip_ = "localhost";
    auto s = helper::ws::GetBestNIC();
    ip_ = helper::ws::GetIPByIndex(s);
    double text_width = glob::GetWidthOfText(font_test_, port_, 45);
    auto portpos = glm::vec2((windowsize.x / 2.f - text_width * 2.7),
                             windowsize.y * 0.5f + 50);
    auto port = registry_create_server_.create();
    auto& port_field = registry_create_server_.assign<InputComponent>(port);
    port_field.pos = portpos;
    port_field.input_name = "PORT";
    port_field.font_size = 45;
    port_field.text = port_;
    port_field.linked_value = &port_;

    text_width = glob::GetWidthOfText(font_test_, max_players_, 45);
    auto playerspos = glm::vec2((windowsize.x / 2.f - text_width * 9.10f),
                                windowsize.y * 0.5f);
    auto maxplayers = registry_create_server_.create();
    auto& players_field =
        registry_create_server_.assign<InputComponent>(maxplayers);
    players_field.pos = playerspos;
    players_field.input_name = "MAX PLAYERS";
    players_field.font_size = 45;
    players_field.text = max_players_;
    players_field.linked_value = &max_players_;
  }
}

void CreateServerState::Init() {
  engine_->SetCurrentRegistry(&registry_create_server_);
}

void CreateServerState::Update(float dt) {
  auto windowsize = glob::window::GetWindowDimensions();
  auto is_enter = Input::IsKeyPressed(GLFW_KEY_ENTER);
  auto is_escape = Input::IsKeyPressed(GLFW_KEY_ESCAPE);
  if (is_enter) {
    MenuEvent click_event;
    click_event.type = MenuEvent::CLICK;
    menu_dispatcher.trigger(click_event);
    // Do shit
  } else if (is_escape) {
    MenuEvent click_event;
    click_event.type = MenuEvent::CLICK;
    menu_dispatcher.trigger(click_event);
    engine_->ChangeState(StateType::MAIN_MENU);
  }
  double text_width = glob::GetWidthOfText(font_test_, "IP: " + ip_, 45);

  auto gamepos =
      glm::vec2((windowsize.x / 2.f - text_width), windowsize.y * 0.5f + 130);
  auto bgpos = glm::vec2((windowsize.x / 2.f - (561.f / 2.f)),
                         windowsize.y / 2.f - 408.f / 2.f);
  glob::Submit(bg_, bgpos, 1.0);
  glob::Submit(font_test_, gamepos, 45, "IP: " + ip_, glm::vec4(1, 1, 1, 1));
}

void CreateServerState::UpdateNetwork() {}

void CreateServerState::Cleanup() {}
CreateServerState::~CreateServerState() {
  if (started_) {
    helper::ps::KillProcess("Server.exe");
    helper::ps::KillProcess("server.exe");
  }
}
bool is_number(const std::string& s) {
  return !s.empty() && std::find_if(s.begin(), s.end(), [](char c) {
                         return !std::isdigit(c);
                       }) == s.end();
}
/*
        Reee
*/
std::string workingdir() {
  char buf[MAX_PATH + 1];
  GetCurrentDirectoryA(MAX_PATH, buf);
  return std::string(buf) + '\\';
}
void CreateServerState::CreateServer() {
  if (is_number(port_) && !port_.empty() && !max_players_.empty() &&
      is_number(max_players_)) {
    std::string arg = ip_ + " " + port_ + " " + max_players_;
    char* arg2 = (char*)arg.c_str();
    char ownPth[MAX_PATH + 1];
    HMODULE hModule = GetModuleHandle(NULL);
    if (hModule != NULL) {
      GetModuleFileName(hModule, ownPth, (sizeof(ownPth)));
      STARTUPINFO si;
      PROCESS_INFORMATION pi;

      std::string path;
      path.resize(300);
      memcpy(path.data(), ownPth, MAX_PATH + 1);
      // memcpy(path.data(), ownPth, strlen(ownPth));
      // path += "server.exe";
      std::string p;
      if (path.find_last_of('\\') != std::string::npos) {
        size_t index = path.find_last_of('\\');
        p = path = path.substr(0, index + 1);
        path += "server.exe";
      }

      helper::ps::KillProcess("server.exe");
      helper::ps::KillProcess("Server.exe");
      ZeroMemory(&si, sizeof(si));
      si.cb = sizeof(si);

      ZeroMemory(&pi, sizeof(pi));
      if (!CreateProcess(
              path.c_str(),      // Filename
              arg2,              // args
              NULL,              // Process handle not inheritable
              NULL,              // Thread handle not inheritable
              FALSE,             // Set handle inheritance to FALSE
              DETACHED_PROCESS,  // Create own process
              NULL,              // Use parent's environment block
              (char*)p.c_str(),  // Working dir.. Not working (huehue)
              &si,               // Pointer to STARTUPINFO structure
              &pi)) {
        std::cout << "Failed to start server " << std::endl;
        return;
      } else {
        started_ = true;
        auto& client = engine_->GetClient();
        client.Connect(ip_.c_str(), std::stoi(port_.c_str()));
        if (!client.IsConnected()) {
          std::cout << "Failed to connect to server" << std::endl;
          return;
        } else {
          engine_->ChangeState(StateType::LOBBY);
        }
      }
    }
  }
}
