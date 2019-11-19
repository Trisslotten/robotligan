#include <GLFW/glfw3.h>
#include <glob/window.hpp>
#include "state.hpp"
#include "../ecs/components.hpp"
#include "../ecs/systems/animation_system.hpp"
#include "engine.hpp"
#include "entitycreation.hpp"

void CreateServerState::Startup()
{
	glob::window::SetMouseLocked(false);
	font_test_ = glob::GetFont("assets/fonts/fonts/ariblk.ttf");
	ButtonComponent* b_c = GenerateButtonEntity(registry_create_server_, "BACK",
		glm::vec2(70, 50), font_test_);
	b_c->button_func = [&]() {
		engine_->ChangeState(StateType::MAIN_MENU);
	};
	
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
		glob::ModelHandle model_map = glob::GetModel("assets/MapV3/Map_Walls.fbx");
		glob::ModelHandle model_map_floor =
			glob::GetModel("assets/MapV3/Map_Floor.fbx");
		glob::ModelHandle model_map_projectors =
			glob::GetModel("assets/MapV3/Map_Projectors.fbx");

		// glob::GetModel(kModelPathMapSingular);
		auto& model_c = registry_create_server_.assign<ModelComponent>(arena);
		model_c.handles.push_back(model_arena);
		model_c.handles.push_back(model_arena_banner);
		model_c.handles.push_back(model_map_projectors);

		registry_create_server_.assign<TransformComponent>(arena, zero_vec, zero_vec,
			arena_scale);

		arena = registry_create_server_.create();
		auto& model_c2 = registry_create_server_.assign<ModelComponent>(arena);
		model_c2.handles.push_back(model_map);
		model_c2.handles.push_back(model_map_floor);
		registry_create_server_.assign<TransformComponent>(arena, zero_vec, zero_vec,
			arena_scale);
	}
	{
		auto arena = registry_create_server_.create();
		glob::ModelHandle model_map_walls =
			glob::GetTransparentModel("assets/MapV3/Map_EnergyWall.fbx");

		auto& model_c = registry_create_server_.assign<ModelComponent>(arena);
		model_c.handles.push_back(model_map_walls);
		registry_create_server_.assign<TransformComponent>(arena, zero_vec, zero_vec,
			arena_scale);
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
	auto& cam_trans = registry_create_server_.assign<TransformComponent>(camera);
	cam_trans.position = glm::vec3(28.f, 2.0f, 0.f);
	glm::vec3 dir = glm::vec3(0) - cam_trans.position;
	cam_c.orientation = glm::quat(glm::vec3(0.f, 0.f, 0.f));
}

void CreateServerState::Init()
{
	engine_->SetCurrentRegistry(&registry_create_server_);
}

void CreateServerState::Update(float dt)
{
	auto windowsize = glob::window::GetWindowDimensions();
	auto is_enter = Input::IsKeyPressed(GLFW_KEY_ENTER);
	auto is_escape = Input::IsKeyPressed(GLFW_KEY_ESCAPE);
	if (is_enter) {
		MenuEvent click_event;
		click_event.type = MenuEvent::CLICK;
		menu_dispatcher.trigger(click_event);
		//Do shit
	}
	else if (is_escape) {
		MenuEvent click_event;
		click_event.type = MenuEvent::CLICK;
		menu_dispatcher.trigger(click_event);
		engine_->ChangeState(StateType::MAIN_MENU);
	}
}

void CreateServerState::UpdateNetwork()
{
}

void CreateServerState::Cleanup()
{
}
