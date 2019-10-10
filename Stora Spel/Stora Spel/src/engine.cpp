#include "engine.hpp"

#include <GLFW/glfw3.h>
#include <bitset>
#include <glm/gtx/transform.hpp>
#include <glob/graphics.hpp>
#include <iostream>

#include <glob\window.hpp>
#include <shared\pick_up_component.hpp>
#include "ecs/components.hpp"
#include "ecs/systems/button_system.hpp"
#include "ecs/systems/render_system.hpp"
#include "entitycreation.hpp"
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

  // Tell the GlobalSettings class to do a first read from the settings file
  GlobalSettings::Access()->UpdateValuesFromFile();

  // glob::GetModel("Assets/Mech/Mech_humanoid_posed_unified_AO.fbx");

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

  main_menu_state_.Startup();
  connect_menu_state_.Startup();
  lobby_state_.Startup();

  play_state_.Startup();

  main_menu_state_.Init();
  current_state_ = &main_menu_state_;
  wanted_state_type_ = StateType::MAIN_MENU;
}

void Engine::Update(float dt) {
  // std::cout << "current message: " << Input::GetCharacters() <<"\n";

  if (take_game_input_ == true) {
    // accumulate key presses
    for (auto const& [key, action] : keybinds_)
      if (Input::IsKeyDown(key)) key_presses_[key]++;
    for (auto const& [button, action] : mousebinds_)
      if (Input::IsMouseButtonDown(button)) mouse_presses_[button]++;

    // accumulate mouse movement
    float mouse_sensitivity = 0.003f;
    glm::vec2 mouse_movement = mouse_sensitivity * Input::MouseMov();
    accum_yaw_ -= mouse_movement.x;
    accum_pitch_ -= mouse_movement.y;

    if (Input::IsKeyPressed(GLFW_KEY_K)) {
      new_team_ = TEAM_BLUE;
    }
    if (Input::IsKeyPressed(GLFW_KEY_L)) {
      new_team_ = TEAM_RED;
    }
  }

  // TODO: move to playstate
  if (current_state_->Type() == StateType::PLAY) {
    glob::Submit(font_test3_, glm::vec2(582, 705), 72,
                 std::to_string(scores_[1]), glm::vec4(0, 0.26, 1, 1));
    glob::Submit(font_test3_, glm::vec2(705, 705), 72,
                 std::to_string(scores_[0]), glm::vec4(1, 0, 0, 1));

  }
  
  current_state_->Update();

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
        current_state_ = &play_state_;
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
    if (presses > 0) actions.set(action, true);
    presses = 0;
  }
  for (auto const& [button, action] : mousebinds_) {
    auto& presses = mouse_presses_[button];
    if (presses > 0) actions.set(action, true);
    presses = 0;
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
      scores_[team] = score;
      break;
    }
    case PacketBlockType::MATCH_TIMER: {
      packet >> gameplay_timer_sec_;
      packet >> countdown_timer_sec_;
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
      // TODO: maybe move, is hack now
      if (current_state_->Type() == StateType::PLAY) {
        play_state_.SwitchGoals();
      }
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
      play_state_.CreatePickUp(pos);
      break;
    }
    case PacketBlockType::DESTROY_PICK_UP: {
      int id;
      packet >> id;
      if (current_state_->Type() == StateType::PLAY) {
        auto pick_up_view = registry_current_->view<PickUpComponent>();
        for (auto entity : pick_up_view) {
          if (id == pick_up_view.get(entity).id) {
            registry_current_->destroy(entity);
          }
        }
      }
      break;
    }
    case PacketBlockType::RECEIVE_PICK_UP: {
      packet >> second_ability_;
      break;
    }
    case PacketBlockType::LOBBY_UPDATE_TEAM: {
      lobby_state_.HandleUpdateLobbyTeamPacket(packet);
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
        case ProjectileID::FORCE_PUSH_OBJECT: {
          play_state_.CreateForcePushObject(e_id);
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
  }
}

void Engine::Render() { glob::Render(); }

void Engine::SetCurrentRegistry(entt::registry* registry) {
  this->registry_current_ = registry;
}

void Engine::UpdateChat(float dt) {
  if (enable_chat_) {
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
      if (chat.IsTakingChatInput() == true &&
          chat.GetCurrentMessage().size() == 0)
        glob::Submit(font_test2_, glm::vec2(50.f, 600.f), 20, "Enter message",
                     glm::vec4(1, 1, 1, 1));
    }
    if (Input::IsKeyPressed(GLFW_KEY_ENTER) && !chat.IsVisable()) {
      // glob::window::SetMouseLocked(false);
      chat.SetShowChat();
    }
    take_game_input_ = !chat.IsTakingChatInput();
    // TODO fix
    // take_game_input_ = !chat.IsTakingChatInput() &&
    // !show_in_game_menu_buttons_;
  }
}

void Engine::UpdateSystems(float dt) {
  UpdateChat(dt);

  if (Input::IsKeyDown(GLFW_KEY_TAB) &&
      current_state_->Type() == StateType::PLAY) {
    DrawScoreboard();
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

void Engine::DrawScoreboard() {
  glob::Submit(gui_scoreboard_back_, glm::vec2(285, 177), 0.6, 100);
  int red_count = 0;
  int blue_count = 0;
  int jump = -16;
  glm::vec2 start_pos_blue = glm::vec2(320, 430);
  glm::vec2 start_pos_red = glm::vec2(320, 320);
  glm::vec2 offset_goals = glm::vec2(140, 0);
  glm::vec2 offset_points = glm::vec2(300, 0);
  if (current_state_->Type() == StateType::PLAY) {
    for (auto& p_score : player_scores_) {
      if (p_score.second.team == TEAM_BLUE) {
        glm::vec2 text_pos = start_pos_blue + glm::vec2(0, blue_count * jump);
        glob::Submit(font_test2_, text_pos, 32, player_names_[p_score.first],
                     glm::vec4(0, 0, 1, 1));
        glob::Submit(font_test2_, text_pos + offset_goals, 32,
                     std::to_string(p_score.second.goals),
                     glm::vec4(0, 0, 1, 1));
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
                     std::to_string(p_score.second.goals),
                     glm::vec4(1, 0, 0, 1));
        glob::Submit(font_test2_, text_pos + offset_points, 32,
                     std::to_string(p_score.second.points),
                     glm::vec4(1, 0, 0, 1));
        red_count++;
      }
    }
  }
}

int Engine::GetGameplayTimer() const { return gameplay_timer_sec_; }

int Engine::GetCountdownTimer() const { return countdown_timer_sec_; }
