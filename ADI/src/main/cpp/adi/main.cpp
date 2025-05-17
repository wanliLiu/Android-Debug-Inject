#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <csignal>
#include <sys/user.h>
#include <cstring>
#include <set>
#include <fstream>
#include <sys/signalfd.h>
#include <sys/syscall.h>
#include <bits/glibc-syscalls.h>
#include <elf.h>
#include <thread>
#include "json.hpp"
#include "InjectProc.h"
#include "logging.h"
#include <getopt.h>  // 必须包含此头文件

using namespace std;
using json = nlohmann::json;

#define WPTEVENT(x) (x >> 16)


inline const char* sigabbrev_np(int sig) {
    if (sig > 0 && sig < NSIG) return sys_signame[sig];
    return "(unknown)";
}




[[noreturn]]
void PtraceTask(){
    InjectProc & injectProc = InjectProc::getInstance();
    pid_t tracd_pid = injectProc.getTracePid();
    ptrace(PTRACE_SEIZE, tracd_pid, 0, PTRACE_O_TRACEFORK);
    int status;
    while(true){
        int pid = waitpid(-1, &status, __WALL);
        if (tracd_pid == -1) {
            continue;
        } else if(tracd_pid == 0){
            continue;
        }
        if(tracd_pid == pid){

            if (WIFEXITED(status) || WIFSIGNALED(status)) {
                LOGE("ptrace process exited\n");
//                kill(getpid(),SIGINT);
                continue;
            }
            if (STOPPED_WITH(status,SIGTRAP, PTRACE_EVENT_FORK)) {
                long child_pid;
                ptrace(PTRACE_GETEVENTMSG, pid, 0, &child_pid);
                LOGD("int fork monitor : %ld\n",child_pid);

            } else if (STOPPED_WITH(status,SIGTRAP, PTRACE_EVENT_STOP) ) {
                if (ptrace(PTRACE_DETACH, pid, 0, 0) == -1)
                    LOGE("failed to detach init\n");
                LOGE("stop tracing init\n");
                continue;
            }

            if (WIFSTOPPED(status)) {

                if (WPTEVENT(status) == 0) {
                    if (WSTOPSIG(status) != SIGSTOP && WSTOPSIG(status) != SIGTSTP && WSTOPSIG(status) != SIGTTIN && WSTOPSIG(status) != SIGTTOU) {
                        LOGD("recv signal : %s %d\n",sigabbrev_np(WSTOPSIG(status)),WSTOPSIG(status));
                        ptrace(PTRACE_CONT, pid, 0, WSTOPSIG(status));
                        continue;
                    } else {
                        LOGD("suppress stopping signal sent to init: %s %d\n",sigabbrev_np(WSTOPSIG(status)), WSTOPSIG(status));
                    }
                }
                ptrace(PTRACE_CONT, pid, 0, 0);
            }

        } else{

            std::set<pid_t> &process = injectProc.get_Tracee_Process();
            auto state = process.find(pid);
            if (state == process.end()) {  //运行到这里说明都是子进程信号,所以要么是新创建的子进程,要么是符合条件的子进程
                // 新创建的子进程会会加入到监控队列,如果是旧的子进程,会走else的分支
                LOGD("new process attached %d",pid);
                process.emplace(pid);
                //前面ptrace的时候,使用的是PTRACE_O_TRACEFORK,所以子进程会在调用fork以后停止,并被追踪到
                ptrace(PTRACE_SETOPTIONS, pid, 0, PTRACE_O_TRACEEXEC); //这段代码 进程会停止在exec加载完,但是还没没有执行的时候
                ptrace(PTRACE_CONT, pid, 0, 0);
                continue;
            }else{
                //旧的子继承,等待他执行完exec,这个时候只是加载了可执行文件,我们可以判断是那个进程了.
                //所以在这里停止,如果在前面停止,我们很难知道要运行的进程是那个.

                //然后通过文件判断是否符合过滤的进程要求,
                LOGD("old process attached %d",pid);
                if (STOPPED_WITH(status,SIGTRAP, PTRACE_EVENT_EXEC)){
                    kill(pid, SIGSTOP);             // 信号会在进程运行起来以后接受到
                    ptrace(PTRACE_CONT, pid, 0, 0); //由于进程当前已经停止,所以先运行起来
                    waitpid(pid, &status, __WALL);
                    if (STOPPED_WITH(status,SIGSTOP, 0)) {   //这个就是接受到的信号,前面 kill(pid, SIGSTOP);  发送的
                        injectProc.monitor_process(pid);
                        ptrace(PTRACE_DETACH, pid, 0, 0);

                    }
                } else {
                    LOGE("old process handle: STOPPED_WITH is not");
                }

                process.erase(state);
                if (WIFSTOPPED(status)) {
                    LOGE("detach process");
                    ptrace(PTRACE_DETACH, pid, 0, 0);
                }
            }
        }
    }
}


void clean_trace(int arg) {
    LOGE("clean_trace ");
    InjectProc & injectProc = InjectProc::getInstance();
    std::set<pid_t> &process = injectProc.get_Tracee_Process();
    for (auto pid:process){
        LOGD("clean_trace detach pid: %d",pid);
        ptrace(PTRACE_DETACH, pid, nullptr, nullptr);
    }
    LOGD("clean_trace trace pid: %d",injectProc.getTracePid());
    ptrace(PTRACE_DETACH, injectProc.getTracePid(), nullptr, nullptr);
    exit(0);
}
int inject_main(){

}

int tracee_main_cmd(pid_t tracee_pid){
    pid_t traced_pid = tracee_pid;
}
int tracee_main_config(char * file){
    std::ifstream f(file);
    json jsonData = nlohmann::json::parse(f);
    InjectProc & injectProc = InjectProc::getInstance();
    json array = jsonData["childProcess"];
    pid_t traced_pid = jsonData.value("traced_pid",-1);
    if(traced_pid <0){
        LOGD("traced_pid is error");
        return 0;
    }
    if(!array.is_array()){
        LOGD("config File is error");
        LOGD("childProcess is not array");
        return 0;
    }
    for(const auto& e : array){
        std::string exec = e.value("exec", "");
        std::string waitSoPath = e.value("waitSoPath", "");
        std::string waitFunSym = e.value("waitFunSym", "");
        std::string InjectSO = e.value("InjectSO", "");
        std::string InjectFunSym = e.value("InjectFunSym", "");
        std::string InjectFunArg = e.value("InjectFunArg", "");
        unsigned int monitorCount = e.value("monitorCount", 0);
        auto cp = ContorlProcess {exec, waitSoPath, waitFunSym, InjectSO, InjectFunSym,InjectFunArg,monitorCount};
        injectProc.add_childProces(cp);
    }

    injectProc.setTracePid(traced_pid);
    std::thread ptraceThread(PtraceTask);
    ptraceThread.join();
}

typedef struct {
    bool help;          // --help 或 -h
    bool verbose;       // --verbose 或 -v
    bool tracee;
    bool inject;
    bool pid;
    bool soPath;
    char *config;       // --output=<file> 或 -o <file>
    int count;          // --count=<num> 或 -c <num>
    char **files;       // 剩余的非选项参数
    int file_count;     // 非选项参数的数量
} ProgramArgs;


void parse_args(int argc, char **argv, ProgramArgs *args) {
    // 初始化默认值
    args->help = false;
    args->verbose = false;
    args->config = NULL;
    args->count = 0;
    args->files = NULL;
    args->file_count = 0;

    // 定义长选项（long options）
    static struct option long_options[] = {
            {"help",    no_argument,       0, 'h'},  // --help
            {"tracee",    no_argument,       0, 't'},  // --tracee
            {"inject",    no_argument,       0, 'i'},  // --inject
            {"config",    required_argument,       0, 'c'},  // --inject
            {"verbose", no_argument,       0, 'v'},  // --verbose
            {"pid",  required_argument, 0, 'p'},  // --pid
            {"output",  required_argument, 0, 'o'},  // --output=<file>
            {"count",   required_argument, 0, 'c'},  // --count=<num>
            {0, 0, 0, 0}  // 结束标记
    };

    // 定义短选项（short options）
    const char *short_options = "hvo:c:";

    int opt;
    int option_index = 0;

    // 解析选项
    while ((opt = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1) {
        switch (opt) {
            case 'h':
                args->help = true;
                break;
            case 'v':
                args->verbose = true;
                break;
            case 't':
                args->tracee = true;
                break;
            case 'i':
                args->inject = true;
                break;
            case 'c':
                args->config = strdup(optarg);  // 复制参数
                break;
            case '?':
                fprintf(stderr, "Unknown option: %s\n", argv[optind - 1]);
                exit(EXIT_FAILURE);
            case ':':
                fprintf(stderr, "Option requires an argument: %s\n", argv[optind - 1]);
                exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char *argv[]) {
    LOGD("buile time: %s",__TIMESTAMP__);
    signal(SIGINT, clean_trace);

    ProgramArgs args;
    parse_args(argc, argv, &args);

    if(args.tracee){
        if(args.config != NULL){
            tracee_main_config(args.config);
        } else{
            tracee_main_cmd();
        }

    }



    return 0;
}

//int main(int argc, char *argv[]) {
//    func_test(argc,argv);
//    return 0;
//}