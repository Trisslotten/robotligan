#include <ecs/components.hpp>
#include <engine.hpp>
#include <entitycreation.hpp>
#include <glob/window.hpp>
#include "state.hpp"
#include <util/global_settings.hpp>
#include <GLFW/glfw3.h>

void SettingsState::Startup() {
  font_test_ = glob::GetFont("assets/fonts/fonts/ariblk.ttf");
}

void SettingsState::Init() {
  glob::window::SetMouseLocked(false);
  engine_->SetSendInput(false);
  engine_->SetCurrentRegistry(&registry_settings_);
  CreateSettingsMenu();
  setting_volume_ = GlobalSettings::Access()->ValueOf("SOUND_VOLUME");
  setting_fov_ = GlobalSettings::Access()->ValueOf("GRAPHICS_FOV");
}

void SettingsState::Update() {
  // top title
  glob::Submit(font_test_,
               glm::vec2(40, glob::window::GetWindowDimensions().y - 20), 72,
               "SETTINGS");
  // graphics
  glob::Submit(font_test_,
               glm::vec2(35, glob::window::GetWindowDimensions().y - 60), 48,
               "GRAPHICS");
  glob::Submit(font_test_,
               glm::vec2(600, glob::window::GetWindowDimensions().y - 60), 48,
               "SOUND");

  if (Input::IsKeyPressed(GLFW_KEY_ESCAPE)) {
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

  glm::vec2 graphics_start_pos =
      glm::vec2(35, glob::window::GetWindowDimensions().y - 175);
  glm::vec2 down_jump = glm::vec2(0, 75);
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
      glm::vec2(600, glob::window::GetWindowDimensions().y - 175);
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
}

void SettingsState::SaveSettings() {
  glob::GetCamera().SetFov(setting_fov_);
  GlobalSettings::Access()->WriteValue("SOUND_VOLUME", setting_volume_);
  GlobalSettings::Access()->WriteValue("GRAPHICS_FOV", setting_fov_);
  engine_->UpdateSettingsValues();
}
