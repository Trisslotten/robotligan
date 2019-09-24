#ifndef ENTITY_CREATION_HPP_
#define ENTITY_CREATION_HPP_

#include <entt.hpp>
#include <glm/glm.hpp>

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

#endif // ENTITY_CREATION_HPP_