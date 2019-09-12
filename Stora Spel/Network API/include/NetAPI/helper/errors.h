#pragma once
#include <windows.h>
#include <NetAPI/common.h>
#include <string>
namespace NetAPI
{
	namespace Helper
	{
		std::string printWSAError(int err);
	}
}

