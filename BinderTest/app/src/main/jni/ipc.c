//
// Created by furture on 2018/2/27.
//
#include "ipc.h"
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <android/log.h>
#include <stdatomic.h>
#include <time.h>
#include <pthread.h>


//#define LOGE(TAG,...) __android_log_print(ANDROID_LOG_ERROR, TAG,__VA_ARGS__)
#define LOGW(TAG,...) __android_log_print(ANDROID_LOG_ERROR, TAG,__VA_ARGS__)
#define LOGE(TAG,...) {}


#define SHARE_MMAP_SIZE   (4*1024*1024)

#define IPC_PAGE_SIZE   (16)

#define IPC_PAGE_OFFSET   (SHARE_MMAP_SIZE/(PAGE_SIZE*sizeof(int)))


#define  IPC_FINISHED    (1)


static JavaVM*  jvm;
static u_int32_t* clientMap;
static u_int32_t* serverMap;
static jclass jBridgeClazz;
static pid_t  childPid;
static pid_t  parentTid;

typedef struct ThreadData {
    int fd;
    bool enableTrace;
} ThreadData;


static double now_ms(void) {
    struct timespec res;
    clock_gettime(CLOCK_REALTIME, &res);
    return 1000.0 * res.tv_sec + (double) res.tv_nsec / 1e6;
}

static void clientIpc();
void native_ipc(JNIEnv* env, jclass this, jbyteArray array);


static  JNINativeMethod methods [] ={
        {
                "ipc","([B)V",  native_ipc
        },
        NULL
};

void native_ipc(JNIEnv* env, jclass this, jbyteArray array){
    double start = now_ms();
    for(int times=0; times < 1000; times++) {
        clientIpc();
    }
    LOGW("ipc", "ipc clientIpc Used  %f ms  %d ", (now_ms() - start), gettid());
}

JNIEXPORT void JNICALL Java_com_furture_myapplication_NativeIpc_ipc(JNIEnv* env, jclass this, jbyteArray array){
    native_ipc(env, this, array);
}



static void lockPage(volatile _Atomic uint32_t * page, pid_t tid){
    LOGE("weex", "ipc try lock thread id %d  %d  %d ", tid, *page, tid|FUTEX_WAITERS);
    uint32_t expected = 0;
    if(atomic_compare_exchange_strong(page, &expected, tid)){
        LOGE("weex", "ipc atomic_compare_exchange_strong lock success ");
        return;
    }
    LOGE("weex", "ipc atomic_compare_exchange_strong failed use futex lock %d  %d %d"
    , tid, *page, FUTEX_WAITERS);
    while (true) {
        int futexReturn = __futex(page, FUTEX_LOCK_PI, 1, NULL);
        if (futexReturn == -1 && errno != EINTR) {
            if(errno ==  EDEADLK){
                LOGE("ipc", "futex lock pi success has get lock before, none need lock %d", gettid());
                exit(0);
            }else{
                LOGE("ipc", "futex lock pi failed, should exit %d %d  %d", futexReturn, errno, *page);
                exit(0);
            }
        }
        //*page
        if(futexReturn == 0){
            LOGE("ipc", "futex lock pi success %d %d %d %u  %d", gettid(), futexReturn, errno, *page, FUTEX_WAITERS);
        }
        if (futexReturn == -1)
            continue;
        break;
    }
    if ((tid & FUTEX_OWNER_DIED)) {
        LOGE("ipc", "original owner has die");
    }
    LOGE("weex", "ipc lock success ");
}

static void unlockPage(volatile _Atomic uint32_t* page, pid_t tid, char* tag){
    uint32_t expected = tid;
    LOGE("ipc", "ipc atomic_compare_exchange_strong unlock old %d  tid %d "
            "%d", *page,tid, expected | FUTEX_WAITERS);
    if (atomic_compare_exchange_strong(page, &expected, 0)) {
        LOGE("weex", "ipc atomic_compare_exchange_strong unlockPage success %s ", tag);
        return;
    }
    if ( (FUTEX_WAITERS| tid) == (expected | FUTEX_WAITERS)) {
        LOGE("ipc", "ipc try unlock futex  %d", tid);
        int futexReturn = __futex(page, FUTEX_UNLOCK_PI, 0, NULL);
        if (futexReturn == -1) {
            LOGE("ipc", "ipc failed to futex unlock %s", strerror(errno));
            return;
        }
        LOGE("ipc", "ipc try unlock futex success  %d  %s", tid, tag);
        return;
    } else if ((expected & FUTEX_TID_MASK) != tid) {
        LOGE("ipc", "ipc error is not equal to tid: %d %d %s", tid, (expected & FUTEX_TID_MASK), tag);
        return;
    }
    LOGE("ipc", "ipc unexpected lock value  %d  %ud %d  %d", tid, *page
    , *page&FUTEX_WAITERS, FUTEX_WAITERS);
}





static void clientIpc(){
    static int readPage = 1;
    static int writePage = 0;
    pid_t tid = gettid();

        LOGE("ipc","ipc client weex write page %d", writePage);
        volatile _Atomic uint32_t*  writePagePtr = (volatile _Atomic uint32_t*)(clientMap + writePage*IPC_PAGE_OFFSET);
        //lockPage(writePagePtr, tid);
        uint32_t* writeData = (uint32_t*)writePagePtr + 2;
        writeData[0] = 1;
        writePagePtr = (volatile _Atomic uint32_t*)(clientMap + writePage*IPC_PAGE_OFFSET);

        //

        writePage +=2;
        writePage = writePage & (IPC_PAGE_SIZE - 1);
        lockPage((volatile _Atomic uint32_t*)(clientMap + writePage*IPC_PAGE_OFFSET), tid);
        unlockPage(writePagePtr, tid, "clientWriteUnlock");

        LOGE("ipc","ipc client weex write data success0 %d %d  %d  %d", tid, readPage, writePage, readPage*IPC_PAGE_OFFSET);
        volatile _Atomic uint32_t* readPagePtr = (volatile _Atomic uint32_t*)(clientMap + readPage*IPC_PAGE_OFFSET);
        LOGE("ipc","ipc client weex write data success1 %d %d  %d  %d", tid, readPage, writePage, readPage*IPC_PAGE_OFFSET);

         LOGE("ipc","ipc client wait read data %d  %d", tid, *readPagePtr);

         lockPage(readPagePtr, tid);
         LOGE("ipc","ipc client lock read data success %d", tid);

        uint32_t* data = (uint32_t*)readPagePtr + 2;
        uint32_t value = data[0];
        LOGE("ipc","ipc client lock read data successdone to unlock %d", value);
        unlockPage(readPagePtr, tid, "clientReadUnlock");
        //call ipc handler
        //write message back
        LOGE("ipc","weex client read data back success");


        readPage +=2;
        readPage = readPage & (IPC_PAGE_SIZE - 1);



}



static void ipcServerLoop(int fd){
    void* map = mmap(NULL, SHARE_MMAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (-1 == fd) {
        LOGE("ipc","ipcServerLoop failed to create ashmem region: %s", strerror(errno));
        return;
    }

    int readPage = 0;
    int writePage = 1;
    pid_t tid = gettid();
    serverMap = map;
    lockPage((volatile _Atomic  uint32_t*)(serverMap + writePage*IPC_PAGE_OFFSET), tid);
    LOGE("ipc","ipcServerLoop write lock success %d  %d  %d  %d %d",
         tid, *(serverMap + writePage*IPC_PAGE_OFFSET), writePage*IPC_PAGE_OFFSET, parentTid, gettid());
    for(int i=1; i<IPC_PAGE_SIZE; i+=2){
        serverMap[i*IPC_PAGE_OFFSET + 1]  |= IPC_FINISHED;
    }

    while (true){
        LOGE("ipc","ipc server loop page %d", readPage);
        volatile _Atomic uint32_t* readPagePtr = (volatile _Atomic uint32_t*)(serverMap + readPage*IPC_PAGE_OFFSET);
        LOGE("ipc","ipc server loop read try lock");
        lockPage(readPagePtr, tid);
        uint32_t* data = (uint32_t*)readPagePtr + 2;
        memcpy(data, "4444", 3);
        unlockPage(readPagePtr, tid, "ServerReadUnlock");
        LOGE("ipc","ipc server read data from client process success");
        //call ipc handler
        //write message back
        volatile _Atomic  uint32_t*  writePagePtr = (volatile _Atomic  uint32_t*)(serverMap + writePage*IPC_PAGE_OFFSET);

        LOGE("ipc","ipc server lock write success");

        uint32_t* writeData = (uint32_t*)writePagePtr + 2;
        writeData[0] = 1;
        writePagePtr = (volatile _Atomic  uint32_t*)(serverMap + writePage*IPC_PAGE_OFFSET);
        LOGE("ipc","ipc server weex write data success");

        writePage +=2;
        writePage = writePage&(IPC_PAGE_SIZE-1);
        lockPage((volatile _Atomic  uint32_t*)(serverMap + writePage*IPC_PAGE_OFFSET), tid);
        unlockPage(writePagePtr, tid, "ServerWriteUnlock");
        LOGE("ipc","ipc server weex write unlock write success");


        readPage +=2;
        readPage = readPage&(IPC_PAGE_SIZE-1);


        LOGE("ipc","ipc server weex write start next loop");
    }
}

static void threadEntry(void* data){
    ThreadData* td = (ThreadData*)data;
    ipcServerLoop(td->fd);
}


void startServer(){
    pid_t  pid = 0;
    LOGE("ipc", "weex ipc ashmem_create_region");
    int fd = ashmem_create_region("WEEX_IPC", 4*1024*1024);
    if (-1 == fd) {
        LOGE("ipc","failed to create ashmem region: %s", strerror(errno));
        return;
    }
    void* map = mmap(NULL, SHARE_MMAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) {
        int _errno = errno;
        close(fd);
        LOGE("ipc", "failed to map ashmem region: %s", strerror(_errno));
        return;
    }
    LOGE("ipc", "weex ipc mmap success %d ", gettid());
    parentTid = gettid();

    clientMap = map;
    for(int i=0; i<IPC_PAGE_SIZE; i+=2){
        LOGE("ipc", "ipc offset %d", i*IPC_PAGE_OFFSET);
        clientMap[i*IPC_PAGE_OFFSET + 1] |= IPC_FINISHED;
    }

    LOGE("ipc", "weex ipc mmap lock page");
    lockPage((volatile _Atomic uint32_t * )map, parentTid);



    pid = fork();
    if(pid < 0){
        LOGE("ipc", "fork failed");
        return;
    }
    if(pid == 0){
        //child
        usleep(100);
        LOGE("weex", "ipc get tid child %d", gettid());
        pthread_attr_t threadAttr;
        pthread_attr_init(&threadAttr);
        pthread_attr_setstacksize(&threadAttr, 10 * 1024 * 1024);
        pthread_t thread;

        ThreadData td = {fd,  false};
        pthread_create(&thread, &threadAttr, threadEntry, &td);
        void* rdata;
        pthread_join(thread, &rdata);

        //ipcServerLoop(fd);
        close(fd);
        exit(0);
    }else{
        LOGE("weex", "ipc get tid parent %d  %d %d %d", gettid(), pid, getpid());
        close(fd);
        volatile uint32_t  childInitFinish = clientMap[1*IPC_PAGE_OFFSET + 1];
        LOGE("ipc","weex ipc server start child loop %d  %d %d  %ld", childInitFinish, gettid(),
         childPid, pthread_self());
        while ((childInitFinish & IPC_FINISHED) == 0){
            sched_yield();
            childInitFinish = clientMap[1*IPC_PAGE_OFFSET + 1];
            LOGE("ipc","ipc yield wait server start");
        }
        childPid = pid;
        //parent
        LOGE("ipc", "weex ipc server start success");
    }
}






JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved){
    LOGE("ipc", "weex ipc jni onload");
    jvm  = vm;
    JNIEnv* env;
    if((*vm)->GetEnv(vm, (void**)&env,JNI_VERSION_1_4) != JNI_OK){
        LOGE("ipc", "weex ipc jni onload get env error");
    }

    jclass tempClass = (*env)->FindClass(env, "com/furture/myapplication/NativeIpc");
    jBridgeClazz = (jclass)(*env)->NewGlobalRef(env, tempClass);
    (*env)->DeleteLocalRef(env, tempClass);

    /**
    jint result = (*env)->RegisterNatives(env,jBridgeClazz, methods, sizeof(methods) / sizeof(methods[0]));
    if(result < 0){
        LOGE("ipc", "weex ipc RegisterNatives error");
        return JNI_FALSE;
    }*/
    LOGE("ipc", "weex ipc jni onload start ipc server");
    startServer();
    return JNI_VERSION_1_4;
}

JNIEXPORT void JNICALL
Java_com_furture_myapplication_NativeIpc_exit(JNIEnv *env, jclass type) {
    munmap(clientMap, SHARE_MMAP_SIZE);
    kill(childPid, SIGKILL);
}

