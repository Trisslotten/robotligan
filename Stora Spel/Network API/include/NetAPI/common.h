#ifndef COMMON_H
#define COMMON_H
#ifdef MAKEDLL_NET
#  define EXPORT __declspec(dllexport)
#else
#  define EXPORT __declspec(dllimport)
#endif
namespace NetAPI
{
	namespace Common
	{
		static const char* socket_not_connected = "!socket_not_connected!";
		static const char* failed_to_recieve = "!FAILED_TO_RECIEVE!";
		static const char* no_data_available = "!NO_DATA_AVAILABLE!";
	}
}
#endif // !COMMON_H
