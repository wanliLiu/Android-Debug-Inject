//
// Created by chic on 2024/12/4.
//

#include <sys/prctl.h>
#include <cstdio>
#include <sys/stat.h>
#include <stdlib.h>
#include <linux/fs.h>
#include "RootImp.h"
#include "logging.h"
# define LOG_TAG "zygiskRootImp"


//bool exec_command(char *restrict buf, size_t len, const char *restrict file, char *const argv[]) {
//    int link[2];
//    pid_t pid;
//
//    if (pipe(link) == -1) {
//        LOGE("pipe: %s\n", strerror(errno));
//
//        return false;
//    }
//
//    if ((pid = fork()) == -1) {
//        LOGE("fork: %s\n", strerror(errno));
//
//        close(link[0]);
//        close(link[1]);
//
//        return false;
//    }
//
//    if (pid == 0) {
//        dup2(link[1], STDOUT_FILENO);
//        close(link[0]);
//        close(link[1]);
//
//        execv(file, argv);
//
//        LOGE("execv failed: %s\n", strerror(errno));
//        _exit(1);
//    } else {
//        close(link[1]);
//
//        ssize_t nbytes = read(link[0], buf, len);
//        if (nbytes > 0) buf[nbytes - 1] = '\0';
//            /* INFO: If something went wrong, at least we must ensure it is NULL-terminated */
//        else buf[0] = '\0';
//
//        wait(NULL);
//
//        close(link[0]);
//    }
//
//    return true;
//}

int RootImp::getProcessFlags(uid_t uid){
    int flag = 0;
    if(uid_is_manager(uid)){
        flag|=PROCESS_IS_MANAGER;
    } else{
        if(uid_granted_root(uid)) {
            flag|=PROCESS_GRANTED_ROOT;
        }
        if (uid_should_umount(uid)) {
            flag |= PROCESS_ON_DENYLIST;
        }
    }
    return flag | getRootFlags(uid);
}




int RootImp::is_command_available(const char *command) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "which %s", command);  // 使用 which 命令检查命令是否存在
    FILE *fp = popen(cmd, "r");
    if (fp == NULL) {
        return 0;  // 如果打开管道失败，返回 0
    }

    char result[256];
    if (fgets(result, sizeof(result), fp) != NULL) {
        // 如果读取到结果，表示命令存在
        fclose(fp);
        return 1;
    } else {
        // 如果没有读取到结果，表示命令不存在
        fclose(fp);
        return 0;
    }
}

bool RootImp::uid_is_manager(uid_t uid) const {
    return uid == this->manager_uid;
}



bool RootImp::is_magisk_root(){
    int version=0;

    if (is_command_available("magisk")) {
        FILE *fp = popen("magisk -V", "r");
        if (fp == NULL) {
            return false;  // 如果打开管道失败，返回 0
        }
        char result[256];
        if (fgets(result, sizeof(result), fp) != NULL) {
            // 如果读取到结果，表示命令存在
            fclose(fp);
            version = atoi(result);
        } else {
            // 如果没有读取到结果，表示命令不存在
            fclose(fp);
            return false;
        }
    } else {
        printf("Magisk command is not available.\n");
        return false;
    }
    if(version <= 0){
        return false;
    } else {
        return true;

    }

    return false;
}

