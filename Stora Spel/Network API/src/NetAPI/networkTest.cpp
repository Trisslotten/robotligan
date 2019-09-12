#include <NetAPI/common.h>
#include <NetAPI/networkTest.h>
#include <NetAPI/helper/netinitialization.h>
namespace NetAPI
{
	EXPORT int testfunc()
	{
		return 0;
	}
	EXPORT bool netInitialized()
	{
		return NetAPI::initialization::GlobalSocketInternals::GetInstance().internals.error;
	}
}