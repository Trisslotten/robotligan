#ifndef STATE_HPP_
#define STATE_HPP_
#include <NetAPI/socket/server.hpp>
#include <entity/registry.hpp>
#include <entt.hpp>
#include "ecs/components.hpp"
class State
{
public:
	virtual void Init(entt::registry& registry, NetAPI::Socket::Server& server) = 0;
	virtual void Update(entt::registry& registry, NetAPI::Socket::Server& server) = 0;
	State() = default;
	~State() {}
private:
};
class LobbyState : public State
{
public:
	void Init(entt::registry& registry, NetAPI::Socket::Server& server);
	void Update(entt::registry& registry, NetAPI::Socket::Server& server);
	LobbyState() = default;
	~LobbyState() {}
private:

};
class PlayState : public State
{
public:
	virtual void Init(entt::registry& registry, NetAPI::Socket::Server& server);
	virtual void Update(entt::registry& registry, NetAPI::Socket::Server& server);
	PlayState() = default;
	~PlayState() { }
private:
};
#endif // !STATE_HPP_
