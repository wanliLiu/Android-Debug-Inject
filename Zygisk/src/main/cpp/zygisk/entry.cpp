#include "daemon.h"
#include "logging.h"
#include "zygisk.hpp"
#include "module.hpp"
#include <dlfcn.h>

using namespace std;


extern "C" [[gnu::visibility("default")]]
void entry(void* handle, const char* path) {
    zygiskComm::InitRequestorSocket(path);
    if (!zygiskComm::PingHeartbeat()) {
        LOGE("Zygisk daemon is not running");
        return;
    }

//#ifdef NDEBUG
//    logging::setfd(zygiskd::RequestLogcatFd());
//#endif

    LOGD("Start hooking");

    hook_entry(handle);
}
