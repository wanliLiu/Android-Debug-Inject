//
// Created by chic on 2025/5/17.
//


#pragma once


enum {
    OPT_SET_DB = 1000,
    OPT_SET_SOCKET,
};

#include <sys/types.h>

struct ProgramArgs {
    bool help;          // --help 或 -h
    bool verbose;       // --verbose 或 -v
    bool start_daemon;

    bool exe_sqlite;
    char *sql;
    bool set_sqlite_db_path;
    char *sqlite_db_path;

    bool set_unix_socket;
    char *unix_socket_path;


    ProgramArgs() {
        help = false;
        verbose = false;
        start_daemon = false;
        exe_sqlite = false;

        set_sqlite_db_path = false;
        sqlite_db_path = "";
        sql = "";
        set_unix_socket = false;
        unix_socket_path = "";

    }
};


void parse_args(int argc, char **argv, ProgramArgs *args);