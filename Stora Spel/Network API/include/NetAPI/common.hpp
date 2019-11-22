#ifndef COMMON_H
#define COMMON_H
#ifdef MAKEDLL_NET
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __declspec(dllimport)
#endif
//typedef unsigned char byte;

namespace NetAPI {
namespace Common {
struct EXPORT netcommands {};
static const char* kSocketNotConnected = "!socket_not_connected!";
static const char* kFailedToRecieve = "!FAILED_TO_RECIEVE!";
static const char* kNoDataAvailable = "!NO_DATA_AVAILABLE!";
static const long kNumPacketBytes = 2*1024;
}  // namespace Common
namespace Socket {

constexpr unsigned short EVERYONE = 255;
}  // namespace Socket
}  // namespace NetAPI
#endif  // !COMMON_H
