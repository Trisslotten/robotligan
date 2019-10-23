#include "engine.hpp"

#include <GLFW/glfw3.h>
#include <bitset>
#include <glm/gtx/transform.hpp>
#include <glob/graphics.hpp>
#include <iostream>

#include <glob\window.hpp>
#include <shared\pick_up_component.hpp>
#include "ecs/components.hpp"
#include "ecs/systems/animation_system.hpp"
#include "ecs/systems/gui_system.hpp"
#include "ecs/systems/particle_system.hpp"
#include "ecs/systems/render_system.hpp"
#include "ecs/systems/sound_system.hpp"
#include "entitycreation.hpp"
#include "eventdispatcher.hpp"
#include "shared/camera_component.hpp"
#include "shared/id_component.hpp"
#include "shared/transform_component.hpp"
#include "util/global_settings.hpp"
#include "util/input.hpp"

Engine::Engine() {}

Engine::~Engine() {}

void Engine::Init() {
  glob::Init();
  Input::Initialize();
  sound_system_.Init(this);
  animation_system_.Init(this);

  // Tell the GlobalSettings class to do a first read from the settings file
  GlobalSettings::Access()->UpdateValuesFromFile();

  // glob::GetModel("Assets/Mech/Mech_humanoid_posed_unified_AO.fbx");

  dispatcher.sink<GameEvent>().connect<&SoundSystem::ReceiveGameEvent>(
      sound_system_);
  dispatcher.sink<GameEvent>().connect<&AnimationSystem::ReceiveGameEvent>(
      animation_system_);
  dispatcher.sink<GameEvent>().connect<&PlayState::ReceiveGameEvent>(
      play_state_);

  SetKeybinds();

  scores_.reserve(2);
  scores_.push_back(0);
  scores_.push_back(0);

  /*gameplay_timer_.reserve(2);
  gameplay_timer_.push_back(4);
  gameplay_timer_.push_back(59);*/

  std::vector<std::string> names = {"Bogdan",  "Smibel Gork", "Big King",
                                    "Blorgon", "Thrall",      "Fisken",
                                    "Snabel",  "BOI"};
  for (int i = 0; i < names.size(); i++) {
    player_names_[i] = names[i];
  }

  // TODO: move to states
  gui_scoreboard_back_ =
      glob::GetGUIItem("assets/GUI_elements/Scoreboard_no_players.png");
  font_test_ = glob::GetFont("assets/fonts/fonts/comic.ttf");
  font_test2_ = glob::GetFont("assets/fonts/fonts/ariblk.ttf");
  font_test3_ = glob::GetFont("assets/fonts/fonts/OCRAEXT_2.TTF");

  main_menu_state_.SetEngine(this);
  lobby_state_.SetEngine(this);
  connect_menu_state_.SetEngine(this);
  play_state_.SetEngine(this);
  settings_state_.SetEngine(this);

  main_menu_state_.Startup();
  settings_state_.Startup();
  connect_menu_state_.Startup();
  lobby_state_.Startup();

  play_state_.Startup();

  main_menu_state_.Init();
  current_state_ = &main_menu_state_;
  wanted_state_type_ = StateType::MAIN_MENU;

  UpdateSettingsValues();
}

void Engine::Update(float dt) {
  // std::cout << "current message: " << Input::GetCharacters() <<"\n";
  /*float latency = 0.080f;
  int counter = 0;
  for (auto& time : time_test) {
    time += dt;

        if (time > latency) {
      while (packet_test.front().IsEmpty() == false) {
            HandlePacketBlock(packet_test.front());
      }
      packet_test.pop_front();
      counter++;
        }
  }
  for (int i = 0; i < counter; ++i) time_test.pop_front();*/

  if (take_game_input_ == true) {
    // accumulate key presses
    for (auto const& [key, action] : keybinds_) {
      key_presses_[key] = 0;
    }
    for (auto const& [button, action] : mousebinds_) {
      mouse_presses_[button] = 0;
    }
    for (auto const& [key, action] : keybinds_)
      if (Input::IsKeyDown(key)) key_presses_[key]++;
    for (auto const& [button, action] : mousebinds_)
      if (Input::IsMouseButtonDown(button)) mouse_presses_[button]++;

    // accumulate mouse movement
    float mouse_sensitivity = 0.003f * mouse_sensitivity_;
    glm::vec2 mouse_movement = mouse_sensitivity * Input::MouseMov();
    accum_yaw_ -= mouse_movement.x;
    accum_pitch_ -= mouse_movement.y;

    // constexpr float pi = glm::pi<float>();
    // test_yaw_ -= mouse_movement.x;
    // test_pitch_ -= mouse_movement.y;
    // test_pitch_ = glm::clamp(test_pitch_, -0.49f * pi, 0.49f * pi);

    // play_state_.SetCameraOrientation(test_pitch_, test_yaw_);
    if (Input::IsKeyPressed(GLFW_KEY_K)) {
      new_team_ = TEAM_BLUE;
    }
    if (Input::IsKeyPressed(GLFW_KEY_L)) {
      new_team_ = TEAM_RED;
    }
  }

  current_state_->Update(dt);

  UpdateSystems(dt);

  // check if state changed
  if (wanted_state_type_ != current_state_->Type()) {
    // cleanup old state
    current_state_->Cleanup();

    // set new state
    switch (wanted_state_type_) {
      case StateType::MAIN_MENU:
        current_state_ = &main_menu_state_;
        break;
      case StateType::CONNECT_MENU:
        current_state_ = &connect_menu_state_;
        break;
      case StateType::LOBBY:
        current_state_ = &lobby_state_;
        break;
      case StateType::PLAY:
        scores_[0] = 0;
        scores_[1] = 0;
        current_state_ = &play_state_;
        break;
      case StateType::SETTINGS:
        current_state_ = &settings_state_;
        break;
    }
    // init new state
    current_state_->Init();
  }

  Input::Reset();
}

void Engine::UpdateNetwork() {
  current_state_->UpdateNetwork();

  // get and send player input
  std::bitset<PlayerAction::NUM_ACTIONS> actions;
  for (auto const& [key, action] : keybinds_) {
    auto& presses = key_presses_[key];
    if (presses > 0) {
      play_state_.AddAction(action);
      actions.set(action, true);
    }
  }
  for (auto const& [button, action] : mousebinds_) {
    auto& presses = mouse_presses_[button];
    if (presses > 0) actions.set(action, true);
  }

  uint16_t action_bits = actions.to_ulong();

  NetAPI::Common::Packet to_send;

  if (!packet_.IsEmpty()) {
    to_send << packet_;
  }

  // message
  if (message_.size() > 0) {
    to_send.Add(message_.c_str(), message_.size());
    to_send << message_.size();
    to_send << PacketBlockType::MESSAGE;
    message_.clear();
  }
  /*
  TODO: fix
  // choose new team
  if (new_team_ != std::numeric_limits<unsigned int>::max()) {
    to_send << new_team_;
    to_send << my_id_;
    to_send << PacketBlockType::CHOOSE_TEAM;

    new_team_ = std::numeric_limits<unsigned int>::max();
  }
  */

  if (should_send_input_) {
    to_send << action_bits;
    to_send << accum_pitch_;
    to_send << accum_yaw_;
    to_send << PacketBlockType::INPUT;
    play_state_.AddAction(100);
    play_state_.SetPitchYaw(accum_pitch_, accum_yaw_);
  } else {
    play_state_.ClearActions();
  }
  if (client_.IsConnected() && !to_send.IsEmpty()) {
    client_.Send(to_send);
  }
  accum_yaw_ = 0.f;
  accum_pitch_ = 0.f;
  packet_ = NetAPI::Common::Packet();

  // handle received data
  if (client_.IsConnected()) {
    auto packets = client_.Receive();
    // std::cout <<"Num recevied packets: "<< packets.size() << "\n";
    for (auto& packet : packets) {
      while (!packet.IsEmpty()) {
        // std::cout << "Remaining packet size: " << packet.GetPacketSize() <<
        // "\n";
        HandlePacketBlock(packet);
      }
      // packet_test.push_back(packet);
      // time_test.push_back(0.0f);
    }
  }
}

void Engine::HandlePacketBlock(NetAPI::Common::Packet& packet) {
  int16_t block_type = -1;
  packet >> block_type;
  switch (block_type) {
    case PacketBlockType::TEST_STRING: {
      size_t strsize = 0;
      packet >> strsize;
      std::string str;
      str.resize(strsize);
      std::cout << "Packet Size: " << packet.GetPacketSize() << "\n";
      packet.Remove(str.data(), strsize);
      std::cout << "PACKET: TEST_STRING: '" << str << "'\n";
      break;
    }
    case PacketBlockType::ENTITY_TRANSFORMS: {
      int size = -1;
      packet >> size;
      for (int i = 0; i < size; i++) {
        EntityID id;
        glm::vec3 position;
        glm::quat orientation;
        packet >> id;
        packet >> position;
        packet >> orientation;
        play_state_.SetEntityTransform(id, position, orientation);
      }
      break;
    }
    case PacketBlockType::PHYSICS_DATA: {
      int size = -1;
      packet >> size;
      for (int i = 0; i < size; i++) {
        EntityID id;
        glm::vec3 vel;
        bool is_airborne;
        packet >> id;
        packet >> vel;
        packet >> is_airborne;
        play_state_.SetEntityPhysics(id, vel, is_airborne);
      }
      break;
    }
    case PacketBlockType::CAMERA_TRANSFORM: {
      glm::quat orientation;
      packet >> orientation;
      play_state_.SetCameraOrientation(orientation);
      break;
    }
    case PacketBlockType::GAME_START: {
      int num_players = -1;
      std::vector<EntityID> player_ids;
      EntityID my_id;
      EntityID ball_id;
      int ability_id;
      packet >> ability_id;
      packet >> num_players;
      player_ids.resize(num_players);
      packet.Remove(player_ids.data(), player_ids.size());
      packet >> my_id;
      packet >> ball_id;

      play_state_.SetEntityIDs(player_ids, my_id, ball_id);
      play_state_.SetMyPrimaryAbility(ability_id);

      ChangeState(StateType::PLAY);
      std::cout << "PACKET: GAME_START\n";
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

      chat_.AddMessage(name, message, message_from);
      if (chat_.IsVisable() == false) {
        chat_.SetShowChat();
        chat_.CloseChat();
      } else if (chat_.IsClosing() == true) {
        // resets the closing timer
        chat_.CloseChat();
      }

      break;
    }
    case PacketBlockType::PLAYER_STAMINA: {
      float stamina = 0.f;
      packet >> stamina;
      play_state_.SetCurrentStamina(stamina);
      break;
    }
    case PacketBlockType::PING: {
      int challenge = 0;
      packet >> challenge;
      challenge *= -1;
      NetAPI::Common::Packet send_packet;
      send_packet << challenge << PacketBlockType::PING;
      client_.Send(send_packet);
      break;
    }
    case PacketBlockType::PING_RECIEVE: {
      unsigned length = 0;
      packet >> length;
      client_pings_.resize(length);
      packet.Remove<>(client_pings_.data(), length);
      break;
    }
    case PacketBlockType::TEAM_SCORE: {
      unsigned int score, team;
      packet >> score;
      packet >> team;
      if (scores_[team] != score) {
        play_state_.TestParticles();
      }
      scores_[team] = score;
      break;
    }
    case PacketBlockType::MATCH_TIMER: {
      int time = 0;
      int countdown_time = 0;
      packet >> countdown_time;
      packet >> time;
      packet >> gameplay_timer_sec_;
      packet >> countdown_timer_sec_;
      play_state_.SetMatchTime(time, countdown_time);
      break;
    }
    /*
    TODO: fix
    case PacketBlockType::CHOOSE_TEAM: {
      PlayerID pid;
      unsigned int team;
      packet >> pid;
      packet >> team;
      if (current_state_->Type() == StateType::PLAY) {
        auto player_view = registry_current_->view<PlayerComponent>();
        for (auto entity : player_view) {
          auto& player = player_view.get(entity);
          if (pid == player.id) {
            // TODO: assign team
            break;
          }
        }
      }
      break;
    }
    */
    case PacketBlockType::SWITCH_GOALS: {
      if (current_state_->Type() == StateType::PLAY) {
        packet >> switch_goal_timer_sec_;
        packet >> switch_goal_time_;
      }
      //// TODO: maybe move, is hack now
      // if (current_state_->Type() == StateType::PLAY) {
      //  play_state_.SwitchGoals();
      //}
      break;
    }
    case PacketBlockType::UPDATE_POINTS: {
      PlayerID id;
      EntityID eid;
      int goals, points, assists, saves, ping;
      unsigned int team;
      packet >> assists;
      packet >> saves;
      packet >> eid;
      packet >> goals;
      packet >> points;
      packet >> id;
      packet >> team;

      PlayerStatInfo psbi;
      psbi.goals = goals;
      psbi.points = points;
      psbi.team = team;
      psbi.enttity_id = eid;
      psbi.assists = assists;
      psbi.saves = saves;
      player_scores_[id] = psbi;
      break;
    }
    case PacketBlockType::CREATE_PICK_UP: {
      glm::vec3 pos;
      EntityID id;
      packet >> id;
      packet >> pos;
      play_state_.CreatePickUp(id, pos);
      break;
    }
    case PacketBlockType::DESTROY_PICK_UP: {
      EntityID id;
      packet >> id;
      if (current_state_->Type() == StateType::PLAY) {
        auto pick_up_view = registry_current_->view<PickUpComponent>();
        for (auto entity : pick_up_view) {
          if (id == pick_up_view.get(entity).id) {
            registry_current_->destroy(entity);
            break;
          }
        }
      }
      break;
    }
    case PacketBlockType::RECEIVE_PICK_UP: {
      packet >> second_ability_;
      break;
    }
    case PacketBlockType::GAME_EVENT: {
      GameEvent event;
      packet >> event;
      dispatcher.trigger(event);
      break;
    }
    case PacketBlockType::LOBBY_UPDATE_TEAM: {
      lobby_state_.HandleUpdateLobbyTeamPacket(packet);
      break;
    }
    case PacketBlockType::PLAYER_LOBBY_DISCONNECT: {
      lobby_state_.HandlePlayerDisconnect(packet);
      break;
    }
    case PacketBlockType::LOBBY_YOUR_ID: {
      int id = 0;
      packet >> id;
      lobby_state_.SetMyId(id);
      break;
    }
    case PacketBlockType::CREATE_PROJECTILE: {
      ProjectileID p_id;
      EntityID e_id;
      packet >> p_id;
      packet >> e_id;

      switch (p_id) {
        case ProjectileID::CANNON_BALL: {
          play_state_.CreateCannonBall(e_id);
          break;
        }
        case ProjectileID::TELEPORT_PROJECTILE: {
          play_state_.CreateTeleportProjectile(e_id);
          break;
        }
        case ProjectileID::FORCE_PUSH_OBJECT: {
          play_state_.CreateForcePushObject(e_id);
          break;
        }
        case ProjectileID::MISSILE_OBJECT: {
          play_state_.CreateMissileObject(e_id);
          break;
        }
      }
      break;
    }
    case PacketBlockType::DESTROY_ENTITIES: {
      EntityID id;
      packet >> id;
      play_state_.DestroyEntity(id);
      break;
    }
    case PacketBlockType::GAME_END: {
      play_state_.EndGame();
      // ChangeState(StateType::LOBBY);
      break;
    }
    case PacketBlockType::YOUR_TARGET: {
      EntityID target;
      packet >> target;
      play_state_.SetMyTarget(target);
      break;
    }
    case PacketBlockType::FRAME_ID: {
      int id;
      packet >> id;
      play_state_.UpdateHistory(id);
      break;
    }
  }
}

void Engine::Render() { glob::Render(); }

void Engine::SetCurrentRegistry(entt::registry* registry) {
  this->registry_current_ = registry;
}

void Engine::UpdateChat(float dt) {
  if (enable_chat_) {
    // chat_ code
    if (chat_.IsVisable()) {
      if (Input::IsKeyPressed(GLFW_KEY_ENTER)) {
        if (chat_.IsClosing() == true) {
          chat_.SetShowChat();
        } else {
          chat_.SetSendMessage(true);
          message_ = chat_.GetCurrentMessage();
          chat_.CloseChat();
        }
      }
      chat_.Update(dt);
      chat_.SubmitText(font_test2_);
      if (chat_.IsTakingChatInput() == true &&
          chat_.GetCurrentMessage().size() == 0)
        glob::Submit(font_test2_, chat_.GetPosition() + glm::vec2(0, -20.f * 5),
                     20, "Enter message", glm::vec4(1, 1, 1, 1));
    }
    if (Input::IsKeyPressed(GLFW_KEY_ENTER) && !chat_.IsVisable()) {
      // glob::window::SetMouseLocked(false);
      chat_.SetShowChat();
    }
    take_game_input_ = !chat_.IsTakingChatInput();
    // TODO fix
    // take_game_input_ = !chat_.IsTakingChatInput() &&
    // !show_in_game_menu_buttons_;
  }
}

void Engine::UpdateSystems(float dt) {
  UpdateChat(dt);
  sound_system_.Update(*registry_current_);

  if (Input::IsKeyDown(GLFW_KEY_TAB) &&
      current_state_->Type() == StateType::PLAY) {
    DrawScoreboard();
  }

  gui_system::Update(*registry_current_);
  ParticleSystem(*registry_current_, dt);
  RenderSystem(*registry_current_);
  animation_system_.UpdateAnimations(*registry_current_, dt);
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

void Engine::DrawScoreboard() {
  glm::vec2 scoreboard_pos = glob::window::GetWindowDimensions();
  scoreboard_pos /= 2;
  scoreboard_pos.x -= 290;
  scoreboard_pos.y -= 150;
  glob::Submit(gui_scoreboard_back_, scoreboard_pos, 0.6, 100);
  int red_count = 0;
  int blue_count = 0;
  int jump = -16;
  glm::vec2 start_pos_blue =
      scoreboard_pos + glm::vec2(24, 260);  //::vec2(320, 430);
  glm::vec2 start_pos_red =
      scoreboard_pos + glm::vec2(24, 140);  // glm::vec2(320, 320);
  glm::vec2 offset_goals = glm::vec2(150, 0);
  glm::vec2 offset_assists = glm::vec2(250, 0);
  glm::vec2 offset_saves = glm::vec2(350, 0);
  glm::vec2 offset_points = glm::vec2(450, 0);
  glm::vec2 offset_ping = glm::vec2(500, 0);
  /*
        goals
        assists
        saves
        points
        ping
  */
  if (current_state_->Type() == StateType::PLAY) {
    for (auto& p_score : player_scores_) {
      if (p_score.second.team == TEAM_BLUE) {
        glm::vec2 text_pos = start_pos_blue + glm::vec2(0, blue_count * jump);
        glob::Submit(font_test2_, text_pos, 32, player_names_[p_score.first],
                     glm::vec4(0, 0, 1, 1));
        glob::Submit(font_test2_, text_pos + offset_goals, 32,
                     std::to_string(p_score.second.goals),
                     glm::vec4(0, 0, 1, 1));
        glob::Submit(font_test2_, text_pos + offset_assists, 32,
                     std::to_string(p_score.second.assists),
                     glm::vec4(0, 0, 1, 1));
        glob::Submit(font_test2_, text_pos + offset_saves, 32,
                     std::to_string(p_score.second.saves),
                     glm::vec4(0, 0, 1, 1));
        glob::Submit(font_test2_, text_pos + offset_points, 32,
                     std::to_string(p_score.second.points),
                     glm::vec4(0, 0, 1, 1));
        glob::Submit(font_test2_, text_pos + offset_ping, 32,
                     std::to_string(client_pings_[p_score.first]),
                     glm::vec4(0, 0, 1, 1));
        blue_count++;
      }
      if (p_score.second.team == TEAM_RED) {
        glm::vec2 text_pos = start_pos_red + glm::vec2(0, red_count * jump);
        glob::Submit(font_test2_, text_pos, 32, player_names_[p_score.first],
                     glm::vec4(1, 0, 0, 1));
        glob::Submit(font_test2_, text_pos + offset_goals, 32,
                     std::to_string(p_score.second.goals),
                     glm::vec4(1, 0, 0, 1));
        glob::Submit(font_test2_, text_pos + offset_assists, 32,
                     std::to_string(p_score.second.assists),
                     glm::vec4(1, 0, 0, 1));
        glob::Submit(font_test2_, text_pos + offset_saves, 32,
                     std::to_string(p_score.second.saves),
                     glm::vec4(1, 0, 0, 1));
        glob::Submit(font_test2_, text_pos + offset_points, 32,
                     std::to_string(p_score.second.points),
                     glm::vec4(1, 0, 0, 1));
        glob::Submit(font_test2_, text_pos + offset_ping, 32,
                     std::to_string(client_pings_[p_score.first]),
                     glm::vec4(1, 0, 0, 1));
        red_count++;
      }
    }
  }
}

int Engine::GetGameplayTimer() const { return gameplay_timer_sec_; }

int Engine::GetCountdownTimer() const { return countdown_timer_sec_; }

int Engine::GetSwitchGoalCountdownTimer() const {
  return switch_goal_timer_sec_;
}

int Engine::GetSwitchGoalTime() const { return switch_goal_time_; }
