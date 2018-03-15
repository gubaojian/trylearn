//
// Created by furture on 2018/3/12.
//
#include "JniHelper.h"





static JavaVM*  jvm;
static pthread_key_t jni_env_key;

JNIEnv*  get_java_jni_env(){
    JNIEnv* env;
    env = (JNIEnv *) pthread_getspecific(jni_env_key);
    if(env != NULL){
        return env;
    }
    JavaVMAttachArgs args = {JNI_VERSION_1_4, 0, 0};
    jvm->AttachCurrentThread(&env, &args);
    if(env == NULL){
        LOGE("LiteBrowser","LiteBrowser ScriptEngine AttachCurrentThread  error ");
        return NULL;
    }
    pthread_setspecific(jni_env_key, env);
    return env;
}


void clean_jni_env(void* ptr){
    JNIEnv* env = (JNIEnv*)ptr;
    if(env && env != NULL){
        LOGE("LiteBrowser","LiteBrowser javascript DetachCurrentThread");
        jvm->DetachCurrentThread();
    }
}

void init_jni_helper(JavaVM* vm, JNIEnv *env){
    pthread_key_create (&jni_env_key, clean_jni_env);
    jvm = vm;
}

