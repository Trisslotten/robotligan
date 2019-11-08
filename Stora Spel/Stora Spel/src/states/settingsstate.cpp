#include <GLFW/glfw3.h>
#include <ecs/components.hpp>
#include <engine.hpp>
#include <entitycreation.hpp>
#include <glob/window.hpp>
#include <util/global_settings.hpp>
#include "state.hpp"

void SettingsState::Startup() {
  font_test_ = glob::GetFont("assets/fonts/fonts/ariblk.ttf");
  ws_ = glob::window::GetWindowDimensions();
}

void SettingsState::Init() {
  glob::window::SetMouseLocked(false);
  engine_->SetSendInput(false);
  engine_->SetCurrentRegistry(&registry_settings_);
  setting_volume_ = GlobalSettings::Access()->ValueOf("SOUND_VOLUME");
  setting_fov_ = GlobalSettings::Access()->ValueOf("GRAPHICS_FOV");
  setting_mouse_sens_ = GlobalSettings::Access()->ValueOf("INPUT_MOUSE_SENS");
  setting_username_ = GlobalSettings::Access()->StringValueOf("USERNAME");
  CreateSettingsMenu();
}

void SettingsState::Update(float dt) {
  // top title
  glob::Submit(font_test_,
               glm::vec2(40, glob::window::GetWindowDimensions().y - 20), 72,
               "SETTINGS");
  // graphics
  glob::Submit(font_test_,
               glm::vec2(35, glob::window::GetWindowDimensions().y - 60), 48,
               "GRAPHICS");
  glob::Submit(font_test_,
               glm::vec2(400, glob::window::GetWindowDimensions().y - 60), 48,
               "SOUND");
  glob::Submit(font_test_,
               glm::vec2(1000, glob::window::GetWindowDimensions().y - 60), 48,
               "INPUT");

  if (Input::IsKeyPressed(GLFW_KEY_ESCAPE)) {
    MenuEvent click_event;
    click_event.type = MenuEvent::CLICK;
    menu_dispatcher.trigger(click_event);
    engine_->ChangeState(engine_->GetPreviousStateType());
  } else if (Input::IsKeyPressed(GLFW_KEY_ENTER)) {
    MenuEvent click_event;
    click_event.type = MenuEvent::CLICK;
    menu_dispatcher.trigger(click_event);
    SaveSettings();
  }
  if (applied_) {
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> time_passed = now - time_;
    auto pos = glm::vec2(ws_.x / 2.f - ws_.x * 0.005, ws_.y / 2.8);
    auto passed = (unsigned)(std::floorf(time_passed.count()));
    std::string dots;
    for (auto i = 0; i < passed; i++) {
      dots += ".";
    }
    std::string text = "Saved";
    glob::Submit(font_test_, pos, 45, std::string("Saved") + dots,
                 glm::vec4(0, 1, 1, 1));
    if (passed > 3) applied_ = false;
  }
  if (Input::IsKeyPressed(GLFW_KEY_ENTER)) {
    SaveSettings();
    engine_->ChangeState(engine_->GetPreviousStateType());
  }
}

void SettingsState::UpdateNetwork() {
  //
}
void SettingsState::Cleanup() { registry_settings_.reset(); }

void SettingsState::CreateSettingsMenu() {
  // BACK BUTTON
  ButtonComponent* b_c = GenerateButtonEntity(registry_settings_, "BACK",
                                              glm::vec2(60, 50), font_test_);
  b_c->button_func = [&]() {
    engine_->ChangeState(engine_->GetPreviousStateType());
  };
  // APPLY BUTTON
  b_c = GenerateButtonEntity(registry_settings_, "APPLY", glm::vec2(60, 100),
                             font_test_);
  b_c->button_func = [=]() { SaveSettings(); };
  // OK (APPLY + BACK) BUTTON
  b_c = GenerateButtonEntity(registry_settings_, "SAVE", glm::vec2(60, 150),
                             font_test_);
  b_c->button_func = [=]() {
    SaveSettings();
    engine_->ChangeState(engine_->GetPreviousStateType());
  };

  glm::vec2 graphics_start_pos =
      glm::vec2(35, glob::window::GetWindowDimensions().y - 175);
  glm::vec2 down_jump = glm::vec2(0, 90);
  // fov slider
  auto fov_slider = registry_settings_.create();
  auto& slider_c = registry_settings_.assign<SliderComponent>(fov_slider);
  slider_c.back_tex = glob::GetGUIItem("assets/GUI_Elements/slider_back.png");
  slider_c.front_tex = glob::GetGUIItem("assets/GUI_Elements/slider_front.png");
  slider_c.value = glob::GetCamera().GetFov();
  slider_c.position = graphics_start_pos + down_jump * 0.f;
  slider_c.min_val = 60.f;
  slider_c.max_val = 120.f;
  slider_c.dimensions = glm::vec2(150, 50);
  slider_c.increment = 1.f;
  slider_c.text = "FOV";
  slider_c.value_to_write = &setting_fov_;
  slider_c.font_handle = font_test_;

  glm::vec2 sound_start_pos =
      glm::vec2(400, glob::window::GetWindowDimensions().y - 175);
  // volume slider
  auto volume_slider = registry_settings_.create();
  auto& vol_slider_c =
      registry_settings_.assign<SliderComponent>(volume_slider);
  vol_slider_c.back_tex =
      glob::GetGUIItem("assets/GUI_Elements/slider_back.png");
  vol_slider_c.front_tex =
      glob::GetGUIItem("assets/GUI_Elements/slider_front.png");
  vol_slider_c.value = GlobalSettings::Access()->ValueOf("SOUND_VOLUME");
  vol_slider_c.position = sound_start_pos + down_jump * 0.f;
  vol_slider_c.min_val = 0.f;
  vol_slider_c.max_val = 100.f;
  vol_slider_c.dimensions = glm::vec2(150, 50);
  vol_slider_c.increment = 0.5f;
  vol_slider_c.text = "VOLUME";
  vol_slider_c.value_to_write = &setting_volume_;
  vol_slider_c.font_handle = font_test_;

  glm::vec2 game_start_pos =
      glm::vec2(1000, glob::window::GetWindowDimensions().y - 175);
  // mouse sensitivity
  auto sens_slider = registry_settings_.create();
  auto& sens_slider_c = registry_settings_.assign<SliderComponent>(sens_slider);
  sens_slider_c.back_tex =
      glob::GetGUIItem("assets/GUI_Elements/slider_back.png");
  sens_slider_c.front_tex =
      glob::GetGUIItem("assets/GUI_Elements/slider_front.png");
  sens_slider_c.value = setting_mouse_sens_;
  sens_slider_c.position = game_start_pos + down_jump * 0.f;
  sens_slider_c.min_val = 0.1f;
  sens_slider_c.max_val = 2.1f;
  sens_slider_c.dimensions = glm::vec2(150, 50);
  sens_slider_c.increment = 0.02f;
  sens_slider_c.text = "MOUSE SENSITIVTY";
  sens_slider_c.value_to_write = &setting_mouse_sens_;
  sens_slider_c.font_handle = font_test_;

  // user name
  auto input_entity = registry_settings_.create();
  auto& input = registry_settings_.assign<InputComponent>(input_entity);
  input.font_size = 32;
  input.max_length = 12;
  input.pos = game_start_pos - down_jump * 1.0f;
  input.input_name = "Username";
  input.text = setting_username_;
  input.linked_value = &setting_username_;
}

void SettingsState::SaveSettings() {
  glob::GetCamera().SetFov(setting_fov_);
  GlobalSettings::Access()->WriteValue("SOUND_VOLUME", setting_volume_);
  GlobalSettings::Access()->WriteValue("GRAPHICS_FOV", setting_fov_);
  GlobalSettings::Access()->WriteValue("INPUT_MOUSE_SENS", setting_mouse_sens_);
  GlobalSettings::Access()->StringWriteValue("USERNAME", setting_username_);

  // printf("Username saved: %s \n", setting_username_.c_str());
  engine_->UpdateSettingsValues();
  applied_ = true;
  time_ = std::chrono::high_resolution_clock::now();
}
