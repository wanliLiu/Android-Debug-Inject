#include "daemon.h"
#include "logging.h"
#include "zygisk.hpp"
#include "module.hpp"
#include "clean.h"
using namespace std;

void *self_handle = nullptr;

extern "C" [[gnu::visibility("default")]]
void entry(void* handle, const char* path) {
    self_handle = handle;
    zygiskComm::InitRequestorSocket(path);
    if (!zygiskComm::PingHeartbeat()) {
        LOGE("Zygisk daemon is not running");
        return;
    }

//#ifdef NDEBUG
//    logging::setfd(zygiskd::RequestLogcatFd());
//#endif
    Dl_info dl_info;
    dladdr((void*)entry, reinterpret_cast<Dl_info *>(&dl_info));

//    addr  0x755d921000 size 835584
    LOGD("dl_info1 :%s ",dl_info.dli_fname);
    LOGD("dl_info1 :%p ",dl_info.dli_fbase);

//    size_t size = clean_trace(dl_info.dli_fname, 1, 0, false);
    LOGD("Start hooking");
//    hook_functions(dl_info.dli_fbase,size);
    hook_functions(0,0);
}
