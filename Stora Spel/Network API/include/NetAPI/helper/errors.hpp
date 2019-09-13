#pragma once
#include <windows.h>
#include <NetAPI/common.hpp>
#include <string>
namespace NetAPI {
namespace Helper {
std::string PrintWSAError(int err);
}
}  // namespace NetAPI
