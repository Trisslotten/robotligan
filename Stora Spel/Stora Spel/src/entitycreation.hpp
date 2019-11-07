#ifndef ENTITY_CREATION_HPP_
#define ENTITY_CREATION_HPP_

#include "ecs/components/button_component.hpp"
#include <entt.hpp>
#include <glm/glm.hpp>
#include <glob/graphics.hpp>

ButtonComponent* GenerateButtonEntity(
    entt::registry& reg, std::string text, glm::vec2 pos,
    glob::Font2DHandle f_handle, bool visible = true,
    unsigned int font_size = 72,
    glm::vec4 normal_color = glm::vec4(.8, .8, .8, 1),
    glm::vec4 hover_color = glm::vec4(1, 1, 1, 1));

void CreateEntities(entt::registry& registry, glm::vec3* in_pos_arr,
                    unsigned int in_num_pos);

void ResetEntities(entt::registry& registry, glm::vec3* in_pos_arr,
                   unsigned int in_num_pos);

void AddBallComponents(entt::registry& registry, entt::entity& entity,
                       glm::vec3 in_pos, glm::vec3 in_vel);

void AddArenaComponents(entt::registry& registry, entt::entity& entity);

void AddPlayerComponents(entt::registry& registry, entt::entity& entity);

void AddRobotComponents(entt::registry& registry, entt::entity& entity,
                        glm::vec3 in_pos);

#endif  // ENTITY_CREATION_HPP_