#include "engine.hpp"

#include <GLFW/glfw3.h>
#include <bitset>
#include <glm/gtx/transform.hpp>
#include <glob/graphics.hpp>
#include <iostream>

#include <button_system.hpp>
#include <glob\window.hpp>
#include <render_system.hpp>
#include "Components/ball_component.hpp"
#include "Components/light_component.hpp"
#include "Components/player_component.hpp"
#include "entitycreation.hpp"
#include "shared/camera_component.hpp"
#include "shared/id_component.hpp"
#include "shared/pick_up_component.hpp"
#include "shared/transform_component.hpp"
#include "util/global_settings.hpp"
#include "util/input.hpp"
#include "util/player_settings.hpp"

Engine::Engine() {}

Engine::~Engine() {}

void Engine::Init() {
  glob::Init();
  Input::Initialize();

  // Tell the GlobalSettings class to do a first read from the settings file
  GlobalSettings::Access()->UpdateValuesFromFile();
  PlayerSettings::Access()->UpdateValuesFromFile();

  TestCreateLights();

  font_test_ = glob::GetFont("assets/fonts/fonts/comic.ttf");
  glob::GetModel("Assets/Mech/Mech_humanoid_posed_unified_AO.fbx");

  ///////////////////////////////////////////////////////////////
  // TO BE MOVED
  ///////////////////////////////////////////////////////////////
  font_test2_ = glob::GetFont("assets/fonts/fonts/ariblk.ttf");
  font_test3_ = glob::GetFont("assets/fonts/fonts/OCRAEXT_2.TTF");
  // Create in-game menu background
  in_game_menu_gui_ =
      glob::GetGUIItem("Assets/GUI_elements/ingame_menu_V1.png");
  // Create 2D element
  e2D_test_ = glob::GetE2DItem("assets/GUI_elements/point_table.png");
  e2D_test2_ =
      glob::GetE2DItem("assets/GUI_elements/Scoreboard_no_players.png");

  // Create GUI elementds
  gui_scoreboard_back_ =
      glob::GetGUIItem("assets/GUI_elements/Scoreboard_no_players.png");
  gui_teamscore_ = glob::GetGUIItem("assets/GUI_elements/point_table.png");
  gui_stamina_base_ =
      glob::GetGUIItem("assets/GUI_elements/stamina_bar_base.png");
  gui_stamina_fill_ =
      glob::GetGUIItem("assets/GUI_elements/stamina_bar_fill.png");
  gui_stamina_icon_ =
      glob::GetGUIItem("assets/GUI_elements/stamina_bar_icon.png");
  gui_quickslots_ =
      glob::GetGUIItem("assets/GUI_elements/koncept_abilities.png");
  ///////////////////////////////////////////////////////////////
  // \TO BE MOVED
  ///////////////////////////////////////////////////////////////

  CreateMainMenu();
  CreateSettingsMenu();
  CreateInGameMenu();
  registry_current_ = &registry_mainmenu_;

  CreateInitalEntities();

  SetKeybinds();

  client.Connect("localhost", 1337);
  scores_.reserve(2);
  scores_.push_back(0);
  scores_.push_back(0);

  std::vector<std::string> names = {"Bogdan",  "Smibel Gork", "Big King",
                                    "Blorgon", "Thrall",      "Fisken",
                                    "Snabel",  "BOI"};

  for (int i = 0; i < names.size(); i++) {
    player_names_[i] = names[i];
  }
}

void Engine::CreateInitalEntities() {
  // Arena
  auto arena = registry_gameplay_.create();
  glm::vec3 zero_vec = glm::vec3(0.0f);
  glm::vec3 arena_scale = glm::vec3(1.0f);
  glob::ModelHandle model_arena =
      glob::GetModel("assets/Map_rectangular/map_rextangular.fbx");
  registry_gameplay_.assign<ModelComponent>(arena, model_arena);
  registry_gameplay_.assign<TransformComponent>(arena, zero_vec, zero_vec,
                                                arena_scale);

  // Ball
  auto ball = registry_gameplay_.create();
  glob::ModelHandle model_ball = glob::GetModel("assets/Ball/Ball.fbx");
  registry_gameplay_.assign<ModelComponent>(ball, model_ball);
  registry_gameplay_.assign<TransformComponent>(ball, zero_vec, zero_vec,
                                                glm::vec3(1.0f));
  registry_gameplay_.assign<BallComponent>(ball);

  // Pick-up
  auto pick_up = registry_gameplay_.create();
  glob::ModelHandle model_pick_up =
      glob::GetModel("assets/lowpolydeer/deer.fbx");  // Replace with real model
  registry_gameplay_.assign<ModelComponent>(pick_up, model_pick_up);
  registry_gameplay_.assign<TransformComponent>(
      pick_up, glm::vec3(5.0f, -5.6f, 0.0f), glm::vec3(0.0f, 0.0f, -1.6f),
      glm::vec3(0.002f));
  registry_gameplay_.assign<PickUpComponent>(pick_up);
}

void Engine::Update(float dt) {
  if (take_game_input_ == true) {
    for (auto const& [key, action] : keybinds_)
      if (Input::IsKeyDown(key)) key_presses_[key]++;
    for (auto const& [button, action] : mousebinds_)
      if (Input::IsMouseButtonDown(button)) mouse_presses_[button]++;

    float mouse_sensitivity = 0.003f;
    glm::vec2 mouse_movement = mouse_sensitivity * Input::MouseMov();
    accum_yaw_ -= mouse_movement.x;
    accum_pitch_ -= mouse_movement.y;
  }

  if (Input::IsKeyPressed(GLFW_KEY_K)) {
    new_team_ = TEAM_BLUE;
  }
  if (Input::IsKeyPressed(GLFW_KEY_L)) {
    new_team_ = TEAM_RED;
  }

  glob::Submit(font_test3_, glm::vec2(582, 705), 72, std::to_string(scores_[1]),
               glm::vec4(0, 0.26, 1, 1));
  glob::Submit(font_test3_, glm::vec2(705, 705), 72, std::to_string(scores_[0]),
               glm::vec4(1, 0, 0, 1));

  UpdateSystems(dt);
  Input::Reset();
}

void Engine::UpdateNetwork() {
  {
    // get and send player input
    std::bitset<PlayerAction::NUM_ACTIONS> actions;
    for (auto const& [key, action] : keybinds_) {
      auto& presses = key_presses_[key];
      if (presses > 0) actions.set(action, true);
      presses = 0;
    }
    for (auto const& [button, action] : mousebinds_) {
      auto& presses = mouse_presses_[button];
      if (presses > 0) actions.set(action, true);
      presses = 0;
    }

    uint16_t action_bits = actions.to_ulong();

    NetAPI::Common::Packet packet;
    packet << action_bits;
    packet << accum_pitch_;
    packet << accum_yaw_;
    packet << PacketBlockType::INPUT;

    // message
    if (message_.size() > 0) {
      packet.Add(message_.c_str(), message_.size());
      packet << message_.size();
      packet << PacketBlockType::MESSAGE;
      message_.clear();
    }
    // choose new team
    if (new_team_ != std::numeric_limits<unsigned int>::max()) {
      packet << new_team_;
      packet << my_id;
      packet << PacketBlockType::CHOOSE_TEAM;
      
      new_team_ = std::numeric_limits<unsigned int>::max();
    }


	//TEMP: Start recording replay
    bool temp = Input::IsKeyPressed(GLFW_KEY_P);
    packet << temp;
    packet << PacketBlockType::TEST_REPLAY_KEYS;
	//TEMP

    if (client.IsConnected()) {
      client.Send(packet);
    } else {
      // TODO: go to main menu or something
    }

    accum_yaw_ = 0.f;
    accum_pitch_ = 0.f;
  }

  {
    // handle received data
    for (auto& packet : client.Receive()) {
      while (!packet.IsEmpty()) {
        HandlePacketBlock(packet);
      }
    }

    if (!transforms.empty()) {
      auto view_players =
          registry_gameplay_.view<TransformComponent, PlayerComponent>();
      for (auto player : view_players) {
        auto& trans_c = view_players.get<TransformComponent>(player);
        auto& player_c = view_players.get<PlayerComponent>(player);
        auto trans = transforms[player_c.id];
        trans_c.position = trans.first;
        trans_c.rotation = trans.second;
        /*
        std::cout << trans_c.position.x << ", ";
        std::cout << trans_c.position.y << ", ";
        std::cout << trans_c.position.z << "\n";
        */
      }
      // std::cout << "\n";
      transforms.clear();
    }
  }
}

void Engine::HandlePacketBlock(NetAPI::Common::Packet& packet) {
  int16_t block_type = -1;
  packet >> block_type;
  switch (block_type) {
    case PacketBlockType::CREATE_PLAYER: {
      PlayerID player_id = -1;
      EntityID entity_id = -1;
      // packet >> entity_id;
      packet >> player_id;
      std::cout << "PACKET: CREATE_PLAYER, player_id=" << player_id
                << ", entity_id=" << entity_id << "\n";
      CreatePlayer(player_id, entity_id);
      break;
    }
    case PacketBlockType::SET_CLIENT_PLAYER_ID: {
      packet >> my_id;
      auto view = registry_gameplay_.view<const PlayerComponent>();
      for (auto& player : view) {
        auto& player_c = view.get(player);
        if (player_c.id == my_id) {
          glm::vec3 camera_offset = glm::vec3(0.38f, 0.62f, -0.06f);
          registry_gameplay_.assign<CameraComponent>(player, camera_offset);
          break;
        }
      }
      std::cout << "PACKET: SET_CLIENT_PLAYER_ID id=" << my_id << "\n";
      break;
    }
    case PacketBlockType::TEST_STRING: {
      size_t strsize = 0;
      packet >> strsize;
      std::string str;
      str.resize(strsize);
      packet.Remove(str.data(), strsize);
      std::cout << "PACKET: TEST_STRING: '" << str << "'\n";
      break;
    }
    case PacketBlockType::MESSAGE: {
      unsigned int message_from;
      packet >> message_from;
      size_t strsize = 0;
      packet >> strsize;
      std::string message;
      message.resize(strsize);
      packet.Remove(message.data(), strsize);
      packet >> strsize;
      std::string name;
      name.resize(strsize);
      packet.Remove(name.data(), strsize);

      chat.AddMessage(name, message, message_from);
      if (chat.IsVisable() == false) {
        chat.SetShowChat();
        chat.CloseChat();
      } else if (chat.IsClosing() == true) {
        // resets the closing timer
        chat.CloseChat();
      }

      break;
    }
    case PacketBlockType::BALL_TRANSFORM: {
      registry_gameplay_.view<TransformComponent, BallComponent>().each(
          [&](auto entity, TransformComponent& trans_c, auto ball) {
            packet >> trans_c.position;
            packet >> trans_c.rotation;
          });
      // std::cout << "Ball pos.y=" << ball_pos.y << "\n";
      break;
    }
    case PacketBlockType::PLAYERS_TRANSFORMS: {
      int size = -1;
      packet >> size;
      for (int i = 0; i < size; i++) {
        PlayerID id;
        glm::vec3 position;
        glm::quat orientation;
        packet >> id;
        packet >> position;
        packet >> orientation;
        transforms[id] = std::make_pair(position, orientation);
      }
      break;
    }
    case PacketBlockType::CAMERA_TRANSFORM: {
      glm::quat orientation;
      packet >> orientation;
      registry_gameplay_.view<CameraComponent>().each(
          [&](auto entity, CameraComponent& cam_c) {
            cam_c.orientation = orientation;
          });
      break;
    }
    case PacketBlockType::PLAYER_STAMINA: {
      packet >> stamina_current_;
      break;
    }
    case PacketBlockType::TEAM_SCORE: {
      unsigned int score, team;
      packet >> score;
      packet >> team;
      scores_[team] = score;
      break;
    }
    case PacketBlockType::CHOOSE_TEAM: {
      PlayerID pid;
      unsigned int team;
      packet >> pid;
      packet >> team;

      auto player_view = registry_gameplay_.view<PlayerComponent>();
      for (auto entity : player_view) {
        auto& player = player_view.get(entity);
        if (pid == player.id) {
          // TODO: assign team
          break;
        }
      }

      break;
    }
    case PacketBlockType::SWITCH_GOALS: {
      TransformComponent& blue_light_trans_c =
          registry_gameplay_.get<TransformComponent>(blue_goal_light_);
      TransformComponent& red_light_trans_c =
          registry_gameplay_.get<TransformComponent>(red_goal_light_); 
      glm::vec3 blue_light_pos = blue_light_trans_c.position;
      blue_light_trans_c.position = red_light_trans_c.position;
      red_light_trans_c.position = blue_light_pos;
      break;
    }
    case PacketBlockType::UPDATE_POINTS: {
      PlayerID id;
      int goals, points;
      unsigned int team;
      packet >> goals;
      packet >> points;
      packet >> id;
      packet >> team;

      PlayerScoreBoardInfo psbi;
      psbi.goals = goals;
      psbi.points = points;
      psbi.team = team;
      player_scores_[id] = psbi;
      break;
    }
    case PacketBlockType::CREATE_PICK_UP: {
      glm::vec3 pos;
      packet >> pos;
      CreatePickUp(pos);
      break;
    }
    case PacketBlockType::DESTROY_PICK_UP: {
      int id;
      packet >> id;

      auto pick_up_view = registry_gameplay_.view<PickUpComponent>();
      for (auto entity : pick_up_view) {
        if (id == pick_up_view.get(entity).id) {
          registry_gameplay_.destroy(entity);
        }
      }

      break;
    }
    case PacketBlockType::RECEIVE_PICK_UP: {
      packet >> second_ability_;
      break;
    }
  }
}

void Engine::Render() { glob::Render(); }

void Engine::SetCurrentRegistry(entt::registry* registry) {
  this->registry_current_ = registry;
}

void Engine::UpdateSystems(float dt) {
  if (registry_current_ ==
      &registry_gameplay_) {  // true if the player is in game
    if (Input::IsKeyPressed(GLFW_KEY_ESCAPE) && !show_in_game_menu_buttons_) {
      glob::window::SetMouseLocked(false);
      show_in_game_menu_buttons_ = true;
      UpdateInGameMenu(show_in_game_menu_buttons_);
    } else if (Input::IsKeyPressed(GLFW_KEY_ESCAPE) &&
               show_in_game_menu_buttons_) {
      glob::window::SetMouseLocked(true);
      show_in_game_menu_buttons_ = false;
      UpdateInGameMenu(show_in_game_menu_buttons_);
    }
    if (show_in_game_menu_buttons_) {
      glob::Submit(in_game_menu_gui_, glm::vec2(491, 152), 1.0f);
    }
    // chat code
    if (chat.IsVisable()) {
      if (Input::IsKeyPressed(GLFW_KEY_ENTER)) {
        if (chat.IsClosing() == true) {
          chat.SetShowChat();
        } else {
          chat.SetSendMessage(true);
          message_ = chat.GetCurrentMessage();
          chat.CloseChat();
        }
      }
      chat.Update(dt);
      chat.SubmitText(font_test2_);
	  if (chat.IsTakingChatInput() == true && chat.GetCurrentMessage().size() == 0)
        glob::Submit(font_test2_, glm::vec2(50.f, 600.f), 20,
                     "Enter message", glm::vec4(1, 1, 1, 1));
    }
    if (Input::IsKeyPressed(GLFW_KEY_ENTER) && !chat.IsVisable()) {
      // glob::window::SetMouseLocked(false);
      chat.SetShowChat();
    }
    take_game_input_ = !chat.IsTakingChatInput() && !show_in_game_menu_buttons_;

    // Submit 2D Element TEST
    glob::Submit(e2D_test_, glm::vec3(10.5f, 1.0f, 0.0f), 2, -90.0f,
                 glm::vec3(0, 1, 0));
    glob::Submit(e2D_test_, glm::vec3(-10.5f, 1.0f, 0.0f), 2, 90.0f,
                 glm::vec3(0, 1, 0));
    glob::Submit(e2D_test2_, glm::vec3(0.0f, 1.0f, -7.0f), 7, 0.0f,
                 glm::vec3(1));

    // Show statistics
    if (Input::IsKeyDown(GLFW_KEY_TAB)) {
      DrawScoreboard();
    }

    // Show GUI TEST
    glob::Submit(gui_stamina_base_, glm::vec2(0, 5), 0.85, 100);
    glob::Submit(gui_stamina_fill_, glm::vec2(7, 12), 0.85, stamina_current_);
    glob::Submit(gui_stamina_icon_, glm::vec2(0, 5), 0.85, 100);
    glob::Submit(gui_quickslots_, glm::vec2(7, 50), 0.3, 100);
    glob::Submit(gui_teamscore_, glm::vec2(497, 648), 1, 100);
  }

  button_system::Update(*registry_current_);
  RenderSystem(*registry_current_);
}

void Engine::SetKeybinds() {
  keybinds_[GLFW_KEY_W] = PlayerAction::WALK_FORWARD;
  keybinds_[GLFW_KEY_S] = PlayerAction::WALK_BACKWARD;
  keybinds_[GLFW_KEY_A] = PlayerAction::WALK_LEFT;
  keybinds_[GLFW_KEY_D] = PlayerAction::WALK_RIGHT;
  keybinds_[GLFW_KEY_LEFT_SHIFT] = PlayerAction::SPRINT;
  keybinds_[GLFW_KEY_SPACE] = PlayerAction::JUMP;
  keybinds_[GLFW_KEY_Q] = PlayerAction::ABILITY_PRIMARY;
  keybinds_[GLFW_KEY_E] = PlayerAction::ABILITY_SECONDARY;
  mousebinds_[GLFW_MOUSE_BUTTON_1] = PlayerAction::KICK;
  mousebinds_[GLFW_MOUSE_BUTTON_2] = PlayerAction::SHOOT;
}

void Engine::CreatePlayer(PlayerID player_id, EntityID entity_id) {
  auto entity = registry_gameplay_.create();

  glm::vec3 alter_scale =
      glm::vec3(5.509f - 5.714f * 2.f, -1.0785f, 4.505f - 5.701f * 1.5f);
  glm::vec3 character_scale = glm::vec3(0.1f);

  glob::ModelHandle player_model =
      glob::GetModel("Assets/Mech/Mech_humanoid_posed_unified_AO.fbx");

  registry_gameplay_.assign<IDComponent>(entity, entity_id);
  registry_gameplay_.assign<PlayerComponent>(entity, player_id);
  registry_gameplay_.assign<TransformComponent>(entity, glm::vec3(),
                                                glm::quat(), character_scale);
  registry_gameplay_.assign<ModelComponent>(entity, player_model,
                                            alter_scale * character_scale);
}

void Engine::CreatePickUp(glm::vec3 position) {
  auto pick_up = registry_gameplay_.create();
  glob::ModelHandle model_pick_up =
      glob::GetModel("assets/lowpolydeer/deer.fbx");  // Replace with real model
  registry_gameplay_.assign<ModelComponent>(pick_up, model_pick_up);
  registry_gameplay_.assign<TransformComponent>(
      pick_up, position, glm::vec3(0.0f, 0.0f, -1.6f),
      glm::vec3(0.002f));
  registry_gameplay_.assign<PickUpComponent>(pick_up);
}

void Engine::TestCreateLights() {
  // Create light
  blue_goal_light_ = registry_gameplay_.create();
  registry_gameplay_.assign<LightComponent>(
      blue_goal_light_, glm::vec3(0.1f, 0.1f, 1.0f), 15.f, 0.2f);
  registry_gameplay_.assign<TransformComponent>(
      blue_goal_light_, glm::vec3(12.f, -4.f, 0.f), glm::vec3(0.f, 0.f, 1.f),
      glm::vec3(1.f));

  red_goal_light_ = registry_gameplay_.create();
  registry_gameplay_.assign<LightComponent>(
      red_goal_light_, glm::vec3(1.f, 0.1f, 0.1f), 15.f, 0.f);
  registry_gameplay_.assign<TransformComponent>(
      red_goal_light_, glm::vec3(-12.f, -4.f, 0.f), glm::vec3(0.f, 0.f, 1.f),
      glm::vec3(1.f));
}

void Engine::DrawScoreboard() {
  glob::Submit(gui_scoreboard_back_, glm::vec2(285, 177), 0.6, 100);
  int red_count = 0;
  int blue_count = 0;
  int jump = -16;
  glm::vec2 start_pos_blue = glm::vec2(320, 430);
  glm::vec2 start_pos_red = glm::vec2(320, 320);
  glm::vec2 offset_goals = glm::vec2(140, 0);
  glm::vec2 offset_points = glm::vec2(300, 0);
  for (auto& p_score : player_scores_) {
    if (p_score.second.team == TEAM_BLUE) {
      glm::vec2 text_pos = start_pos_blue + glm::vec2(0, blue_count * jump);
      glob::Submit(font_test2_, text_pos, 32, player_names_[p_score.first],
                   glm::vec4(0, 0, 1, 1));
      glob::Submit(font_test2_, text_pos + offset_goals, 32,
                   std::to_string(p_score.second.goals), glm::vec4(0, 0, 1, 1));
      glob::Submit(font_test2_, text_pos + offset_points, 32,
                   std::to_string(p_score.second.points),
                   glm::vec4(0, 0, 1, 1));
      blue_count++;
    }
    if (p_score.second.team == TEAM_RED) {
      glm::vec2 text_pos = start_pos_red + glm::vec2(0, red_count * jump);
      glob::Submit(font_test2_, text_pos, 32, player_names_[p_score.first],
                   glm::vec4(1, 0, 0, 1));
      glob::Submit(font_test2_, text_pos + offset_goals, 32,
                   std::to_string(p_score.second.goals), glm::vec4(1, 0, 0, 1));
      glob::Submit(font_test2_, text_pos + offset_points, 32,
                   std::to_string(p_score.second.points),
                   glm::vec4(1, 0, 0, 1));
      red_count++;
    }
  }
}
void Engine::CreateMainMenu() {
  glob::window::SetMouseLocked(false);
  font_test_ = glob::GetFont("assets/fonts/fonts/ariblk.ttf");

  // PLAY BUTTON - change registry to registry_gameplay_
  ButtonComponent* b_c = GenerateButtonEntity(registry_mainmenu_, "PLAY",
                                              glm::vec2(100, 200), font_test_);
  b_c->button_func = [&]() {
    this->SetCurrentRegistry(&registry_gameplay_);
    glob::window::SetMouseLocked(true);
  };

  // SETTINGS BUTTON - change registry to registry_settings_
  b_c = GenerateButtonEntity(registry_mainmenu_, "SETTINGS",
                             glm::vec2(100, 140), font_test_);
  b_c->button_func = [&]() { this->SetCurrentRegistry(&registry_settings_); };

  // EXIT BUTTON - close the game
  b_c = GenerateButtonEntity(registry_mainmenu_, "EXIT", glm::vec2(100, 80),
                             font_test_);
  b_c->button_func = [&]() { exit(0); };
}

void Engine::CreateSettingsMenu() {
  // BACK BUTTON in SETTINGS - go back to main menu
  ButtonComponent* b_c = GenerateButtonEntity(registry_settings_, "BACK",
                                              glm::vec2(100, 200), font_test_);
  b_c->button_func = [&]() { this->SetCurrentRegistry(&registry_mainmenu_); };
}

void Engine::CreateInGameMenu() {
  // CONTINUE BUTTON -- change registry to registry_gameplay_
  ButtonComponent* in_game_buttons_ = GenerateButtonEntity(
      registry_gameplay_, "CONTINUE", glm::vec2(550, 430), font_test_, false);
  in_game_buttons_->button_func = [&]() {
    // All the logic here
  };
  // SETTINGS BUTTON -- change registry to registry_settings_
  in_game_buttons_ = GenerateButtonEntity(
      registry_gameplay_, "SETTINGS", glm::vec2(550, 360), font_test_, false);

  in_game_buttons_->button_func = [&] {
    // All the logic here
  };

  // END GAME -- change registry to registry_mainmenu_
  in_game_buttons_ = GenerateButtonEntity(
      registry_gameplay_, "TO LOBBY", glm::vec2(550, 290), font_test_, false);
  in_game_buttons_->button_func = [&] {
    // All the logic here
  };

  in_game_buttons_ = GenerateButtonEntity(
      registry_gameplay_, "EXIT", glm::vec2(550, 220), font_test_, false);
  in_game_buttons_->button_func = [&] { exit(0); };
}

void Engine::UpdateInGameMenu(bool show_menu) {
  // Set in_game buttons visibility
  auto view = registry_gameplay_.view<ButtonComponent, TransformComponent>();
  for (auto v : view) {
      auto& button_c = registry_gameplay_.get<ButtonComponent>(v);
      button_c.visible = show_menu;
    }
  }
