#include "state.hpp"

#include <glob/window.hpp>
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
		"Resting", 0.05f, &animation_c, 10, 1.f,
		engine_->GetAnimationSystem().LOOP);


	auto camera = registry_create_server_.create();
	auto& cam_c = registry_create_server_.assign<CameraComponent>(camera);
	auto& cam_trans = registry_create_server_.assign<TransformComponent>(camera);
	cam_trans.position = glm::vec3(28.f, 1.5f, 0.f);
	glm::vec3 dir = glm::vec3(0) - cam_trans.position;
	cam_c.orientation = glm::quat(glm::vec3(0.f, 0.f, 0.f));
}

void CreateServerState::Init()
{
	engine_->SetCurrentRegistry(&registry_create_server_);
}

void CreateServerState::Update(float dt)
{

}

void CreateServerState::UpdateNetwork()
{
}

void CreateServerState::Cleanup()
{
}
