#pragma once
#include <string>
#include <functional>
#include <glob/graphics.hpp>
struct PopupComponent
{
	bool centered = true;
	glm::vec2 position = glm::vec2(0.0f);
	std::string text = "";
	glob::Font2DHandle f_handle = 0;
	std::function<void> func = nullptr;
	PopupComponent()
	{
		f_handle = glob::GetFont("assets/fonts/fonts/ariblk.ttf");
	}
};