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

    LOGD("Start hooking");
    hook_functions();
    Dl_info dl_info;
    dladdr((void*)hook_functions, reinterpret_cast<Dl_info *>(&dl_info));
    string file_path = dl_info.dli_fname;
    uintptr_t so_start_addr = (uintptr_t)dl_info.dli_fbase;
    size_t so_size = remove_soinfo(file_path.c_str(), 1, 0, false);
    setValue(so_start_addr,so_size);
//    reSoMap(file_path.c_str());
}
