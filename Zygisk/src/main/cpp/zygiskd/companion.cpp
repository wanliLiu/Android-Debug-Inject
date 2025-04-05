//
// Created by chic on 2025/4/5.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>

#include <unistd.h>
#include <linux/limits.h>
#include <pthread.h>
#include <asm-generic/poll.h>
#include "logging.h"
#include "socket_utils.h"
#include <poll.h>
#include "dl.h"
#include "vector"
# define LOG_TAG "zygisk-commpanion"


typedef void (*zygisk_companion_entry)(int);

struct companion_module_thread_args {
    int fd;
    zygisk_companion_entry entry;
};

zygisk_companion_entry load_module(int fd) {
    char path[PATH_MAX];
    snprintf(path, sizeof(path), "/proc/self/fd/%d", fd);

    void *handle = DlopenExt(path, RTLD_NOW);

    if (!handle) return NULL;

    void *entry = dlsym(handle, "zygisk_companion_entry");
    if (!entry) {
        LOGE("Failed to dlsym zygisk_companion_entry: %s\n", dlerror());

        dlclose(handle);

        return NULL;
    }

    return (zygisk_companion_entry)entry;
}

/* WARNING: Dynamic memory based */
void *entry_thread(void *arg) {
    struct companion_module_thread_args *args = (struct companion_module_thread_args *)arg;

    int fd = args->fd;
    zygisk_companion_entry module_entry = args->entry;

    struct stat st0 = { 0 };
    if (fstat(fd, &st0) == -1) {
        LOGE(" - Failed to get initial client fd stats: %s\n", strerror(errno));

        free(args);

        return NULL;
    }

    module_entry(fd);

    /* INFO: Only attempt to close the client fd if it appears to be the same file
               and if we can successfully stat it again. This prevents double closes
               if the module companion already closed the fd.
    */
    struct stat st1;
    if (fstat(fd, &st1) != -1 || st0.st_ino == st1.st_ino) {
        LOGI(" - Client fd changed after module entry\n");

        close(fd);
    }

    free(args);

    return NULL;
}

bool check_unix_socket(int fd, bool block) {
    struct pollfd pfd = {
            .fd = fd,
            .events = POLLIN,
            .revents = 0
    };

    int timeout = block ? -1 : 0;
    poll(&pfd, 1, timeout);

    return pfd.revents & ~POLLIN ? false : true;
}



/* WARNING: Dynamic memory based */
void companion_entry(int socket) {
    if (getuid() != 0 || fcntl(socket, F_GETFD) < 0)
        exit(-1);
   //   给 zygisd ack
    socket_utils::write_u32(socket, 0);

    int client = socket_utils::recv_fd(socket);








}