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
		
	}
}
#endif // !COMMON_H
