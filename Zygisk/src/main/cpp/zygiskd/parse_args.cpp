//
// Created by chic on 2025/5/17.
//

#include "parse_args.h"
#include <getopt.h>  // 必须包含此头文件
#include <cstring>
#include <cstdio>
#include <dirent.h>
#include <cstdlib>
#include "logging.h"

void parse_args(int argc, char **argv, ProgramArgs *args) {
    // 初始化默认值
    int opt;
    int option_index = 0;

    static struct option long_options[] = {

            {"help", no_argument,       0,            'h'},
            {"verbose", no_argument,       0,         'v'},
            {"daemon",    no_argument,       0,       'd'},
            {"config",    required_argument,       0, 'c'},
            {"db",   required_argument, 0,            OPT_SET_DB},
            {"unix_socket",   required_argument, 0,   OPT_SET_SOCKET},
            {"sqlite",   required_argument, 0,        's'},


            {0, 0, 0,                                 0}  // 结束标记
    };

    // 定义短选项（short options）
    const char *short_options = "himvc:p:n:";


    // 解析选项
    while ((opt = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1) {
        switch (opt) {
            case 'd':
                args->start_daemon = true;
                break;
            case 'h':
                args->help = true;
                break;
            case 's':
                args->exe_sqlite = true;
                args->sql = strdup(optarg);

                break;
            case 'v':
                args->verbose = true;
                break;

            case OPT_SET_DB:
                args->set_sqlite_db_path = true;
                args->set_sqlite_db_path = strdup(optarg);
                break;
            case OPT_SET_SOCKET:
                args->set_unix_socket = true;
                args->unix_socket_path = strdup(optarg);
                break;

        }
    }
}