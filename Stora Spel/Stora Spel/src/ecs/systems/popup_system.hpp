#pragma once

#include "../components/popup_component.h"
#include "../components/button_component.hpp"
#include <entt.hpp>
void update(entt::registry& registry)
{
	auto view_popup = registry.view<PopupComponent>();
	for (auto& e : view_popup)
	{
		
	}
}