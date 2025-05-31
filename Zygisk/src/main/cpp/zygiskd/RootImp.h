//
// Created by chic on 2024/12/4.
//

#ifndef ZYGISKNEXT_ROOTIMP_H
#define ZYGISKNEXT_ROOTIMP_H
#include "sys/types.h"




enum : uint32_t {

    PROCESS_GRANTED_ROOT = (1u << 0),
    PROCESS_ON_DENYLIST = (1u << 1),

    PROCESS_IS_MANAGER = (1u << 27),
    PROCESS_ROOT_IS_APATCH = (1u << 28),
    PROCESS_ROOT_IS_KSU = (1u << 29),
    PROCESS_ROOT_IS_MAGISK = (1u << 30),
    IS_FIRST_PROCESS = (1u << 31),

    PRIVATE_MASK = (PROCESS_IS_MANAGER | PROCESS_ROOT_IS_APATCH | PROCESS_ROOT_IS_KSU |
                    PROCESS_ROOT_IS_MAGISK | IS_FIRST_PROCESS),
    UNMOUNT_MASK = PROCESS_ON_DENYLIST
};




class RootImp {


public:
    int getProcessFlags(uid_t uid);
    static bool is_magisk_root();
    static int is_command_available(const char *command);


    RootImp(const RootImp&)= delete;

    RootImp(){
        manager_uid = get_mamager_uid();
    };

    RootImp& operator=(const RootImp)=delete;
    bool uid_is_manager(uid_t uid) const;

    virtual bool uid_should_umount(uid_t uid)  = 0;
    virtual bool uid_granted_root(uid_t uid) = 0 ;
    virtual int getRootFlags (uid_t uid) = 0;

    virtual uid_t get_mamager_uid() = 0;

private:
    uid_t manager_uid = -1;
    int root_imp;

};




#endif //ZYGISKNEXT_ROOTIMP_H
