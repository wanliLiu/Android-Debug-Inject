# Android-Debug-Inject

# description

这是一个android 进程注入项目,可以参考zygiskNext,注入的方式是通过ptrace,以及一些调试相关的知识完成注入,初衷是无痕对抗,最有完成如下功能
+ 注入无痕,针对检测工具的逻辑专门写成了这种方式,目前在我已知检测方式中无痕
+ 可注入zygote以外的其他任何进程,init进程的所有子进程
+ 提供高定制化的注入时机选择,在任何进程的任何一个so加载或者任何一个函数的开始执行的时候提供注入时机
+ zygisk 使用的是zygiskNext 的zygisk 以及magisk 的zygiskd服务,经过测试可以支持目前开源的最有一个lsp版本

# 结构
整个程序分为三部分
+ adi android-debug-inject 注入工具
+ zygisk,提供zygisk 以及zyiskd 服务
+ ADLib, 这是一个注入 drm 进程 的demo,可以修改drm 的id

# 使用
通过adi程序输入配置文件,进行监控

通过这个配置文件,将监控zygote启动,并注入libzygisk.so文件
module/src/zygisk.json

通过这个配置文件,将监控drm进程启动,并注入so文件
ADILib/src/main/cpp/inject.json


# 配置文件参数说明

```json
{  
    "traced_pid": 1,    要监控的父进程
    "persistence": true,    暂时不用,后续可能会做持久化
    "childProcess": [       要监控的进程数组
       {  
          "exec": "/vendor/bin/hw/android.hardware.drm@1.4-service.widevine",    监控的进程exec文件名字
          "waitSoPath": "/apex/com.android.art/lib64/libart.so",                 等待这个so加载在继续执行
          "waitFunSym": "",                 等待这个函数执行在继续执行 
          "InjectSO": "/data/adb/modules/ZygiskADI/lib/arm64-v8a/libDrmHook.so",  要加载的so文件
          "InjectFunSym": "DrmIdHook",         要执行的函数
          "InjectFunArg": "11111111111193baf8cb6a22de8a5ae4bfecc174b1a9405dc71b8b3fac1c734f"    函数参数,目前只支持一个,会传入第二个参数,第一个为so的handle
       }  
  
    ]  
}

waitSoPath 尽量不要不写
waitFunSym 可以不写,如果不写,将在so加载以后直接加载so.


waitSoPath和waitFunSym,一般是是配合,表示某个so的某个函数,但这个函数执行以后执行hook代码

```



## 问题
+ zygisk commpanion 

这个功能并未完全测试,lsp使用虽然测试了,但是lsp并未完全使用,目前不知道这个功能是否有问题,而且这个功能我使用的比较少,抄袭的magisk 的zygisk的代码

+ init 子进程的问题

提供init子进程注入功能,但是目前的zygisk 设计框架可能不一定是适合,比如selinux权限这些
kernelsu 提供的挂在镜像的 /data/adb/module 更是无法支持,init ns里没有挂载这个目录
如果想要使用这个功能建议自己处理挂在和so权限问题,这是比较简单的,而且我觉得是比较正常的.



## 最后
感谢zygiskNext带来开源部分代码,以及magisk 这些项目,也希望大家且看且珍惜,对开源项目多一点宽容




 #  感谢
https://github.com/Dr-TSNG/ZygiskNext
https://github.com/topjohnwu/Magisk