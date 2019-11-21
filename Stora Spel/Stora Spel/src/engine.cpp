#include "engine.hpp"

#include <GLFW/glfw3.h>
#include <bitset>
#include <glm/gtx/transform.hpp>
#include <glob/graphics.hpp>
#include <iostream>

#include <ecs\systems\trail_system.hpp>
#include <glob\window.hpp>
#include <shared\pick_up_component.hpp>
#include "ecs/components.hpp"
#include "ecs/systems/animation_system.hpp"
#include "ecs/systems/gui_system.hpp"
#include "ecs/systems/input_system.hpp"
#include "ecs/systems/lifetime_system.hpp"
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

Engine::~Engine() {
  if (this->replay_machine_ != nullptr) {
    delete this->replay_machine_;
  }
  if (this->registry_replay_ != nullptr) {
    delete this->registry_replay_;
  }
}

void Engine::Init() {
  glob::Init();
  Input::Initialize();
  sound_system_.Init(this);
  animation_system_.Init(this);

  // Tell the GlobalSettings class to do a first read from the settings file
  GlobalSettings::Access()->UpdateValuesFromFile();

  // glob::GetModel("Assets/Mech/Mech_humanoid_posed_unified_AO.fbx");

  menu_dispatcher.sink<MenuEvent>().connect<&SoundSystem::ReceiveMenuEvent>(
      sound_system_);

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
  chat_.SetFont(font_test2_);

  // Initiate the Replay Machine
  unsigned int length_sec =
      (unsigned int)GlobalSettings::Access()->ValueOf("REPLAY_LENGTH_SECONDS");
  unsigned int approximate_tickrate = 64;  // TODO: Replace with better
                                           // approximation
  this->replay_machine_ =
      new ClientReplayMachine(length_sec, approximate_tickrate);
  this->replay_machine_->SetEngine(this);

  dispatcher.sink<GameEvent>().connect<&ClientReplayMachine::ReceiveGameEvent>(
      *replay_machine_);
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

    play_state_.AddPitchYaw(-mouse_movement.y, -mouse_movement.x);

    if (Input::IsKeyPressed(GLFW_KEY_K)) {
      new_team_ = TEAM_BLUE;
    }
    if (Input::IsKeyPressed(GLFW_KEY_L)) {
      new_team_ = TEAM_RED;
    }

    // Replay stuff
    if (Input::IsKeyPressed(GLFW_KEY_I)) {
      if (!this->recording_) {
        this->BeginRecording();
      } else {
        this->StopRecording();
      }
    }
    if (Input::IsKeyPressed(GLFW_KEY_O)) {
      this->SaveRecording();
      std::cout << this->replay_machine_->GetSelectedReplayStringTree() << "\n";
    }
    if (Input::IsKeyPressed(GLFW_KEY_P)) {
      this->BeginReplay();
    }
    // Replay stuff
  }

  // Check if we are in play state
  if (this->current_state_->Type() == StateType::PLAY) {
    // Once we are check if we are recording
    // or if we are replaying
    if (this->recording_) {
      this->replay_machine_->RecordFrame(*(this->registry_current_));
    } else if (this->replaying_) {
      this->PlayReplay();
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
        // std::cout << "CHANGE STATE: MAIN_MENU\n";
        break;
      case StateType::CONNECT_MENU:
        current_state_ = &connect_menu_state_;
        // std::cout << "CHANGE STATE: CONNECT_MENU\n";
        break;
      case StateType::LOBBY:
        current_state_ = &lobby_state_;
        // std::cout << "CHANGE STATE: LOBBY\n";
        break;
      case StateType::PLAY:
        current_state_ = &play_state_;
        // std::cout << "CHANGE STATE: PLAY\n";
        // ReInit();
        scores_[0] = 0;
        scores_[1] = 0;
        break;
      case StateType::SETTINGS:
        current_state_ = &settings_state_;
        // std::cout << "CHANGE STATE: SETTINGS\n";
        break;
    }
    // init new state
    current_state_->Init();
  }

  if (Input::IsKeyPressed(GLFW_KEY_F7)) {
    glob::SetSSAO(true);
  }
  if (Input::IsKeyPressed(GLFW_KEY_F8)) {
    glob::SetSSAO(false);
  }

  if (Input::IsKeyPressed(GLFW_KEY_F5)) {
    glob::ReloadShaders();
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
      // play_state_.AddAction(action);
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
    // play_state_.AddPitchYaw(accum_pitch_, accum_yaw_);
    to_send << action_bits;
    to_send << play_state_.GetPitch();
    to_send << play_state_.GetYaw();
    to_send << PacketBlockType::INPUT;
  } else {
    // play_state_.ClearActions();
  }
  if (client_.IsConnected() && !to_send.IsEmpty()) {
    client_.Send(to_send);
  }
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
      // std::cout << "PACKET: TEST_STRING\n";
      size_t strsize = 0;
      packet >> strsize;
      std::string str;
      str.resize(strsize);
      // std::cout << "Packet Size: " << packet.GetPacketSize() << "\n";
      packet.Remove(str.data(), strsize);
      // std::cout << "PACKET: TEST_STRING: '" << str << "'\n";
      break;
    }
    case PacketBlockType::ENTITY_TRANSFORMS: {
      // std::cout << "PACKET: ENTITY_TRANSFORMS\n";
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
      // std::cout << "PACKET: PHYSICS_DATA\n";
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
      // std::cout << "PACKET: CAMERA_TRANSFORM\n";
      glm::quat orientation;
      packet >> orientation;
      play_state_.SetCameraOrientation(orientation);
      break;
    }
    case PacketBlockType::PLAYER_LOOK_DIR: {
      int num_dirs = -1;
      packet >> num_dirs;
      for (int i = 0; i < num_dirs; i++) {
        EntityID id = 0;
        glm::vec3 look_dir;
        packet >> id;
        packet >> look_dir;
        play_state_.SetPlayerLookDir(id, look_dir);
      }
      break;
    }
    case PacketBlockType::PLAYER_MOVE_DIR: {
      int num_dirs = -1;
      packet >> num_dirs;
      for (int i = 0; i < num_dirs; i++) {
        EntityID id = 0;
        glm::vec3 move_dir;
        packet >> id;
        packet >> move_dir;
        play_state_.SetPlayerMoveDir(id, move_dir);
      }
      break;
    }

    case PacketBlockType::GAME_START: {
      // std::cout << "PACKET: GAME_START\n";
      unsigned int team;
      int num_players = -1;
      std::vector<EntityID> player_ids;
      EntityID my_id;
      EntityID ball_id;
      int ability_id;
      int num_team_ids;
      glm::vec3 arena_scale;
      packet >> arena_scale;
      packet >> ability_id;
      packet >> num_players;
      player_ids.resize(num_players);
      packet.Remove(player_ids.data(), player_ids.size());
      packet >> my_id;
      packet >> ball_id;
      packet >> team;
      play_state_.SetEntityIDs(player_ids, my_id, ball_id);
      play_state_.SetMyPrimaryAbility(ability_id);
      play_state_.SetTeam(team);
      play_state_.SetArenaScale(arena_scale);
      packet >> num_team_ids;
      for (int i = 0; i < num_team_ids; i++) {
        long client_id;
        EntityID e_id;
        unsigned int team;
        packet >> client_id;
        packet >> e_id;
        packet >> team;
        PlayerStatInfo psbi;
        psbi.goals = 0;
        psbi.points = 0;
        psbi.team = team;
        psbi.enttity_id = e_id;
        psbi.assists = 0;
        psbi.saves = 0;
        player_scores_[client_id] = psbi;
      }

      ChangeState(StateType::PLAY);

      std::cout << "PACKET: GAME_START\n";
      break;
    }
    case PacketBlockType::MESSAGE: {
      // std::cout << "PACKET: MESSAGE\n";
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
      // std::cout << "PACKET: PLAYER_STAMINA\n";
      float stamina = 0.f;
      packet >> stamina;
      play_state_.SetCurrentStamina(stamina);
      break;
    }
    case PacketBlockType::PING: {
      // std::cout << "PACKET: PING\n";
      int challenge = 0;
      packet >> challenge;
      challenge *= -1;
      NetAPI::Common::Packet send_packet;
      send_packet << challenge << PacketBlockType::PING;
      client_.Send(send_packet);
      break;
    }
    case PacketBlockType::PING_RECIEVE: {
      // std::cout << "PACKET: PING_RECIEVE\n";
      unsigned length = 0;
      packet >> length;
      client_pings_.resize(length);
      packet.Remove<>(client_pings_.data(), length);
      break;
    }
    case PacketBlockType::TEAM_SCORE: {
      // std::cout << "PACKET: TEAM_SCORE\n";
      unsigned int score, team;
      packet >> score;
      packet >> team;
      scores_[team] = score;
      break;
    }
    case PacketBlockType::MATCH_TIMER: {
      // std::cout << "PACKET: MATCH_TIMER\n";
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
      // std::cout << "PACKET: SWITCH_GOALS\n";
      packet >> switch_goal_timer_;
      packet >> switch_goal_time_;
      break;
    }
    case PacketBlockType::SECONDARY_USED: {
      second_ability_ = AbilityID::NULL_ABILITY;
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
    case PacketBlockType::CREATE_WALL: {
      glm::quat rot;
      glm::vec3 pos;
      EntityID id;

      packet >> id;
      packet >> pos;
      packet >> rot;
      play_state_.CreateWall(id, pos, rot);
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
        auto pick_up_view =
            registry_current_->view<PickUpComponent, IDComponent>();
        for (auto entity : pick_up_view) {
          if (id == pick_up_view.get<IDComponent>(entity).id) {
            registry_current_->destroy(entity);
            break;
          }
        }
      }
      break;
    }
    case PacketBlockType::SERVER_CAN_JOIN: {
      // std::cout << "PACKET: SERVER_CAN_JOIN\n";
      packet >> server_connected_;
      std::cout << server_connected_;
      break;
    }
    case PacketBlockType::SERVER_STATE: {
      // std::cout << "PACKET: STATE\n";
      ServerStateType state;
      packet >> state;
      SetServerState(state);
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
      // std::cout << "PACKET: LOBBY_UPDATE_TEAM\n";
      lobby_state_.HandleUpdateLobbyTeamPacket(packet);
      break;
    }
    case PacketBlockType::PLAYER_LOBBY_DISCONNECT: {
      // std::cout << "PACKET: PLAYER_LOBBY_DISCONNECT\n";
      lobby_state_.HandlePlayerDisconnect(packet);
      break;
    }
    case PacketBlockType::LOBBY_YOUR_ID: {
      // std::cout << "PACKET: LOBBY_YOUR_ID\n";
      int id = 0;
      packet >> id;
      lobby_state_.SetMyId(id);
      break;
    }
    case PacketBlockType::CREATE_PROJECTILE: {
      ProjectileID p_id;
      EntityID e_id;
      glm::vec3 pos;
      glm::quat ori;
      packet >> ori;
      packet >> pos;
      packet >> p_id;
      packet >> e_id;

      switch (p_id) {
        case ProjectileID::CANNON_BALL: {
          play_state_.CreateCannonBall(e_id, pos, ori);
          break;
        }
        case ProjectileID::TELEPORT_PROJECTILE: {
          play_state_.CreateTeleportProjectile(e_id, pos, ori);
          break;
        }
        case ProjectileID::FORCE_PUSH_OBJECT: {
          play_state_.CreateForcePushObject(e_id, pos, ori);
          break;
        }
        case ProjectileID::MISSILE_OBJECT: {
          play_state_.CreateMissileObject(e_id, pos, ori);
          // TODO: Dont trigger this event on the client like this. Fix so that
          // event is sent/received AFTER the create_projectile packet on server
          // instead Note: Sometimes this plays on player entity rather than the
          // missile entity [???]
          GameEvent missile_event;
          missile_event.type = GameEvent::MISSILE_FIRE;
          missile_event.missile_fire.projectile_id = e_id;
          dispatcher.trigger(missile_event);
          break;
        }
        case ProjectileID::BLACK_HOLE: {
          play_state_.CreateBlackHoleObject(e_id, pos, ori);
          break;
		}
      }
      break;
    }
    case PacketBlockType::DESTROY_ENTITIES: {
      EntityID id;
      packet >> id;

      // If we are recording notify replay machine
      // before entity is gone
      if (this->recording_) {
        this->replay_machine_->NotifyDestroyedObject(
            id, *(this->registry_current_));
      }

      // Remove the entity
      play_state_.DestroyEntity(id);

      // Contemplate life
      break;
    }
    case PacketBlockType::GAME_END: {
      // std::cout << "PACKET: GAME_END\n";
      play_state_.EndGame();
      previous_state_ = StateType::LOBBY;
      // ChangeState(StateType::LOBBY);
      break;
    }
    case PacketBlockType::GAME_OVERTIME: {
      play_state_.OverTime();
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
    case PacketBlockType::CREATE_BALL: {
      EntityID id;
      packet >> id;
      play_state_.CreateNewBallEntity(false, id);
      break;
    }
    case PacketBlockType::CREATE_FAKE_BALL: {
      EntityID id;
      packet >> id;
      play_state_.CreateNewBallEntity(true, id);
      break;
    }
    case PacketBlockType::TO_CLIENT_NAME: {
      long client_id;
      size_t name_size = 0;
      std::string name;
      packet >> client_id;
      packet >> name_size;
      name.resize(name_size);
      packet.Remove(name.data(), name.size());
      player_names_[client_id] = name;
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
          if (current_state_ == &play_state_) chat_.CloseChat();
        }
      }
      chat_.Update(dt);
      chat_.SubmitText();
      if (chat_.IsTakingChatInput() == true &&
          chat_.GetCurrentMessage().size() == 0)
        glob::Submit(font_test2_, chat_.GetPosition() + glm::vec2(0, -20.f * 5),
                     28, "Enter message", glm::vec4(1, 1, 1, 1));
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
  input_system::Update(*registry_current_);
  ParticleSystem(*registry_current_, dt);
  animation_system_.UpdateAnimations(*registry_current_, dt);
  trailsystem::Update(*registry_current_, dt);
  lifetime::Update(*registry_current_, dt);
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

std::vector<int>* Engine::GetPlayingPlayers() {
  auto val = play_state_.GetPlayerIDs();
  if (val && !val->empty()) {
    return val;
  } else {
    return nullptr;
  }
}

int Engine::GetGameplayTimer() const { return gameplay_timer_sec_; }

int Engine::GetCountdownTimer() const { return countdown_timer_sec_; }

float Engine::GetSwitchGoalCountdownTimer() const { return switch_goal_timer_; }

int Engine::GetSwitchGoalTime() const { return switch_goal_time_; }

// Replay Functions ---
void Engine::BeginRecording() {
  std::cout << "<Begining to record>" << std::endl;

  // If we are currently not replaying,
  // start recording
  if (!this->replaying_) {
    this->recording_ = true;
  }
}

void Engine::StopRecording() {
  std::cout << "<Stopped recording>" << std::endl;
  // Stop recordng
  this->recording_ = false;
}

void Engine::SaveRecording() {
  std::cout << "<Saved replay>" << std::endl;

  // Tell the ReplayMachine to save what currently lies in its buffer
  this->replay_machine_->StoreReplay();

  // NTS: Currently always selects the latest replay
  // as per the following code
  this->replay_machine_->SelectReplay(
      this->replay_machine_->NumberOfStoredReplays() - 1);
}

void Engine::BeginReplay() {
  std::cout << "<Starting replay>" << std::endl;

  // Stop recording
  this->StopRecording();

  // Swap to the registry of the replay machine
  // if we are not already replaying
  if (!this->replaying_) {
    this->registry_on_hold_ = this->registry_current_;
    this->registry_replay_ = new entt::registry;
    this->play_state_.FetchMapAndArena(*(this->registry_replay_));
    this->registry_current_ = this->registry_replay_;

    this->replaying_ = true;
  }
}

void Engine::PlayReplay() {
  // If we aren't replaying, return
  if (!this->replaying_) {
    return;
  }

  std::cout << "<Replaying>" << std::endl;
  // std::cout << this->replay_machine_->GetSelectedReplayStringState()
  //          << std::endl;

  // Send in registry to get the next frame from the replay machine
  if (this->replay_machine_->LoadFrame(*(this->registry_current_))) {
    // If the recording is not playing
    //	- Either it has ended
    //	- Or it doesn't exist
    // Swap back registries
    this->registry_current_ = this->registry_on_hold_;
    delete this->registry_replay_;
    this->registry_replay_ = nullptr;
    this->registry_on_hold_ = nullptr;

    this->replaying_ = false;

    this->replay_machine_->ResetSelectedReplay();

    std::cout << "<Replay finished>" << std::endl;
    return;
  }

  // Update replay camera
  UpdateReplayCamera();
}

void Engine::UpdateReplayCamera() {
  entt::basic_view camera_view =
      this->registry_replay_->view<CameraComponent, TransformComponent>();
  entt::basic_view target_view =
      this->registry_replay_->view<TargetComponent, TransformComponent>();

  for (entt::entity target : target_view) {
    TargetComponent& target_c = registry_replay_->get<TargetComponent>(target);
    TransformComponent& target_trans_c =
        registry_replay_->get<TransformComponent>(target);

    for (entt::entity camera : camera_view) {
      CameraComponent& cam_c = registry_replay_->get<CameraComponent>(camera);
      TransformComponent& cam_trans_c =
          registry_replay_->get<TransformComponent>(camera);

      // Create vector from camera to target
      glm::vec3 temp_dir =
          glm::normalize(target_trans_c.position - cam_trans_c.position);

      auto quarter_turn = glm::quat(glm::vec3(0, glm::pi<float>() * 0.5f, 0));

      cam_c.orientation =
          glm::quatLookAt(temp_dir, glm::vec3(0, 1, 0)) * quarter_turn;
    }
  }
}

// Replay Functions ---