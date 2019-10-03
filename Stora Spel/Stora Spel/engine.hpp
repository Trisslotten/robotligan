#ifndef ENGINE_HPP_
#define ENGINE_HPP_

#include <NetAPI/socket/Client.hpp>
#include <NetAPI/socket/tcpclient.hpp>
#include <entt.hpp>
#include <glob/graphics.hpp>
#include <unordered_map>
#include "shared/shared.hpp"
#include "Chat.hpp"

class Engine {
 public:
  Engine();
  ~Engine();
  Engine(const Engine&) = delete;
  Engine& operator=(const Engine&) = delete;

  void Init();

  void CreateInitalEntities();

  void Update(float dt);

  void UpdateNetwork();

  void Render();

  void SetCurrentRegistry(entt::registry* registry);
 private:
  void SetKeybinds();

  void UpdateSystems(float dt);
  void HandlePacketBlock(NetAPI::Common::Packet& packet);

  void CreateMainMenu();
  void CreateSettingsMenu();
  void CreateInGameMenu();
  void UpdateInGameMenu(bool show_menu);

  void CreatePlayer(PlayerID player_id, EntityID entity_id);
  void CreatePickUp(glm::vec3 position);
  void TestCreateLights();

  NetAPI::Socket::Client client;

  entt::registry registry_gameplay_;
  entt::registry registry_mainmenu_;
  entt::registry registry_settings_;
  entt::registry* registry_current_;

  PlayerID my_id = -1;

  std::unordered_map<PlayerID, std::pair<glm::vec3, glm::quat>> transforms;

  std::unordered_map<int, int> keybinds_;
  std::unordered_map<int, int> mousebinds_;
  std::unordered_map<int, int> key_presses_;
  std::unordered_map<int, int> mouse_presses_;
  float accum_yaw_ = 0.f;
  float accum_pitch_ = 0.f;
  float stamina_current_ = 0.f;

  glob::Font2DHandle font_test_ = 0;
  glob::Font2DHandle font_test2_ = 0;
  glob::Font2DHandle font_test3_ = 0;
  glob::E2DHandle e2D_test_, e2D_test2_;
  glob::GUIHandle in_game_menu_gui_ = 0;
  glob::GUIHandle gui_test_, gui_teamscore_, gui_stamina_base_,
      gui_stamina_fill_, gui_stamina_icon_, gui_quickslots_;
  bool show_in_game_menu_buttons_ = false;

  std::vector<unsigned int> scores;

  entt::entity blue_goal_light;
  entt::entity red_goal_light;
  bool take_game_input_ = true;
  Chat chat;
  std::string message_ = "";
  AbilityID second_ability_ = AbilityID::NULL_ABILITY;
};

#endif  // ENGINE_HPP_