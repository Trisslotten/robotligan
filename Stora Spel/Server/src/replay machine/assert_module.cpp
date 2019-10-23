#include "assert_module.hpp"

#include <shared/id_component.hpp>
#include <shared/transform_component.hpp>

#include "ecs/components/ability_component.hpp"
#include "ecs/components/player_component.hpp"

#include "util/global_settings.hpp"
#include <iostream>

// Private---------------------------------------------------------------------

bool AssertModule::AssertAbilityComponents(entt::registry& in_registry) {
  // Get all AbilityComponent:s  from the registry
  // affected by the replay
  auto replay_view = in_registry.view<IDComponent, AbilityComponent>();

  // Get all AbilityComponent:s from the assertion
  // registry for this frame
  entt::registry assert_reg;
  this->assert_log_->ReadSnapshot(assert_reg);
  auto assert_view = assert_reg.view<IDComponent, AbilityComponent>();

  // std::cout << "View Sizes:\t" << replay_view.size() << "\t:\t"
  //           << assert_view.size() << "\n";

  // First let's just check if the sizes are the same
  if (replay_view.size() != assert_view.size()) {
    GlobalSettings::Access()->WriteError(
        "replay_machine.cpp", "AssertAbilityComponents()", "View sizes differ");
    return false;
  }

  // If they are we start a for loop accessing each entity
  // in the replay view, retrieve its id component and
  // ability component, find the corresponding id in the
  // assert view and compare their transform components.
  // If they are different write out error.
  for (auto replay_entity : replay_view) {
    IDComponent& replay_id_c = in_registry.get<IDComponent>(replay_entity);
    AbilityComponent& replay_ability_c =
        in_registry.get<AbilityComponent>(replay_entity);

    for (auto assert_entity : assert_view) {
      IDComponent& assert_id_c = assert_reg.get<IDComponent>(assert_entity);
      AbilityComponent& assert_ability_c =
          assert_reg.get<AbilityComponent>(assert_entity);

      if (replay_id_c.id == assert_id_c.id) {
        // std::cout << "ID: " << replay_id_c.id << " -status- "
        //          << (replay_ability_c == assert_ability_c) << "\n";

        if (replay_ability_c != assert_ability_c) {
          GlobalSettings::Access()->WriteError("replay_machine.cpp",
                                               "AssertAbilityComponents()",
                                               "Replay Desync");
          return false;
        }
      }
    }
  }
  return true;
}

bool AssertModule::AssertPlayerComponents(entt::registry& in_registry) {
  // Get all PlayerComponent:s  from the registry
  // affected by the replay
  auto replay_view = in_registry.view<IDComponent, PlayerComponent>();

  // Get all PlayerComponent:s from the assertion
  // registry for this frame
  entt::registry assert_reg;
  this->assert_log_->ReadSnapshot(assert_reg);
  auto assert_view = assert_reg.view<IDComponent, PlayerComponent>();

  // std::cout << "View Sizes:\t" << replay_view.size() << "\t:\t"
  //           << assert_view.size() << "\n";

  // First let's just check if the sizes are the same
  if (replay_view.size() != assert_view.size()) {
    GlobalSettings::Access()->WriteError(
        "replay_machine.cpp", "AssertPlayerComponents()", "View sizes differ");
    return false;
  }

  // If they are we start a for loop accessing each entity
  // in the replay view, retrieve its id component and
  // player component, find the corresponding id in the
  // assert view and compare their transform components.
  // If they are different write out error.
  for (auto replay_entity : replay_view) {
    IDComponent& replay_id_c = in_registry.get<IDComponent>(replay_entity);
    PlayerComponent& replay_player_c =
        in_registry.get<PlayerComponent>(replay_entity);

    for (auto assert_entity : assert_view) {
      IDComponent& assert_id_c = assert_reg.get<IDComponent>(assert_entity);
      PlayerComponent& assert_player_c =
          assert_reg.get<PlayerComponent>(assert_entity);

      if (replay_id_c.id == assert_id_c.id) {
        // std::cout << "ID: " << replay_id_c.id << " -status- "
        //          << (replay_player_c == assert_player_c) << "\n";

        if (replay_player_c != assert_player_c) {
          GlobalSettings::Access()->WriteError("replay_machine.cpp",
                                               "AssertPlayerComponents()",
                                               "Replay Desync");
          return false;
        }
      }
    }
  }
  return true;
}

bool AssertModule::AssertTransformComponents(entt::registry& in_registry) {
  // Get all TransformComponent:s from the registry
  // affected by the replay
  auto replay_view = in_registry.view<IDComponent, TransformComponent>();

  // Get all TransformComponent:s from the assertion
  // registry for this frame
  entt::registry assert_reg;
  this->assert_log_->ReadSnapshot(assert_reg);
  auto assert_view = assert_reg.view<IDComponent, TransformComponent>();

  // std::cout << "View Sizes:\t" << replay_view.size() << "\t:\t"
  //           << assert_view.size() << "\n";

  // First let's just check if the sizes are the same
  if (replay_view.size() != assert_view.size()) {
    GlobalSettings::Access()->WriteError("replay_machine.cpp",
                                         "AssertTransformComponents()",
                                         "View sizes differ");
    return false;
  }

  // If they are we start a for loop accessing each entity
  // in the replay view, retrieve its id component and
  // transform component, find the corresponding id in the
  // assert view and compare their transform components.
  // If they are different write out error.
  for (auto replay_entity : replay_view) {
    IDComponent& replay_id_c = in_registry.get<IDComponent>(replay_entity);
    TransformComponent& replay_transform_c =
        in_registry.get<TransformComponent>(replay_entity);

    for (auto assert_entity : assert_view) {
      IDComponent& assert_id_c = assert_reg.get<IDComponent>(assert_entity);
      TransformComponent& assert_transform_c =
          assert_reg.get<TransformComponent>(assert_entity);

      if (replay_id_c.id == assert_id_c.id) {
        // std::cout << "ID: " << replay_id_c.id << " -status- "
        //           << (replay_transform_c == assert_transform_c) << "\n";

        if (replay_transform_c != assert_transform_c) {
          GlobalSettings::Access()->WriteError("replay_machine.cpp",
                                               "AssertTransformComponents()",
                                               "Replay Desync");
          return false;
        }
      }
    }
  }
  return true;
}

// Public----------------------------------------------------------------------

AssertModule::AssertModule(unsigned int in_num_of_frames) {
  this->assert_log_ = new RegPack(in_num_of_frames);
  int temp = (int)GlobalSettings::Access()->ValueOf("ASSERT_MODE");
  switch (temp) {
    case ASSERT_NOTHING:
      this->mode_ = ASSERT_NOTHING;
      break;
    case ASSERT_ABILITY_COMPONENTS:
      this->mode_ = ASSERT_ABILITY_COMPONENTS;
      break;
    case ASSERT_PLAYER_COMPONENTS:
      this->mode_ = ASSERT_PLAYER_COMPONENTS;
      break;
    case ASSERT_TRANSFORM_COMPONENTS:
      this->mode_ = ASSERT_TRANSFORM_COMPONENTS;
      break;
    case ASSERT_ALL:
      this->mode_ = ASSERT_ALL;
      break;
    default:
      this->mode_ = ASSERT_NOTHING;
      break;
  }

  std::cout << "Assert Module Type <" << this->mode_ << "> created\n";

}

AssertModule::~AssertModule() {
  if (this->assert_log_ != nullptr) {
    delete this->assert_log_;
  }
}

void AssertModule::SnapshotRegistry(entt::registry& in_registry) {
  this->assert_log_->WriteSnapshot(in_registry);
}

bool AssertModule::AssertRegistry(entt::registry& in_registry) {
  bool ret_val = false;

  switch (this->mode_) {
    case ASSERT_NOTHING:
      ret_val = true;
      break;
    case ASSERT_ABILITY_COMPONENTS:
      ret_val = this->AssertAbilityComponents(in_registry);
      break;
    case ASSERT_PLAYER_COMPONENTS:
      ret_val = this->AssertPlayerComponents(in_registry);
      break;
    case ASSERT_TRANSFORM_COMPONENTS:
      ret_val = this->AssertTransformComponents(in_registry);
      break;
    case ASSERT_ALL:
      ret_val = this->AssertAbilityComponents(in_registry);
      ret_val = ret_val && this->AssertPlayerComponents(in_registry);
      ret_val = ret_val && this->AssertTransformComponents(in_registry);
      break;
    default:
      ret_val = false;
      break;
  }

  return ret_val;
}