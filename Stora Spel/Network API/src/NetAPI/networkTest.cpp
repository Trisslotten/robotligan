#include <NetAPI/common.hpp>
#include <NetAPI/helper/netinitialization.hpp>
#include <NetAPI/networkTest.hpp>
namespace NetAPI {
EXPORT int TestFunc() { return 0; }
EXPORT bool NetInitialized() {
  return NetAPI::Initialization::GlobalSocketInternals::GetInstance()
      .internals_.error;
}
}  // namespace NetAPI