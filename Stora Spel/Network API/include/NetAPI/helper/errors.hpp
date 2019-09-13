#include <windows.h>
#include <NetAPI/common.hpp>
#include <string>
#ifndef ERROR_H
#define ERROR_H

namespace NetAPI {
namespace Helper {
std::string PrintWSAError(int err);
}
}  // namespace NetAPI
#endif // !ERROR_H