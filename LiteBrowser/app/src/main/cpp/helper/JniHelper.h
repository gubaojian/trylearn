//
// Created by furture on 2018/3/12.
//

#ifndef LITEBROWSER_JNIHELP_H
#define LITEBROWSER_JNIHELP_H

#include <jni.h>
#include <pthread.h>
#include <android/log.h>

#define LOGE(TAG,...) __android_log_print(ANDROID_LOG_ERROR, TAG,__VA_ARGS__)


#define DEBUG_LOG(...) __android_log_print(ANDROID_LOG_ERROR, "LiteBrowser",__VA_ARGS__)


JNIEnv*  get_java_jni_env();

void clean_jni_env(void* ptr);

void init_jni_helper(JavaVM* vm, JNIEnv *env);



#endif //LITEBROWSER_JNIHELP_H
