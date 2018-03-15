//
// Created by furture on 2018/3/12.
//

#include "ScopedJObject.h"
#include "JniHelper.h"
#include "../litebrowsercommon.h"


ScopedJObject::ScopedJObject(jobject thisObject){
    JNIEnv* env = get_java_jni_env();
    _thisObject = env->NewGlobalRef(thisObject);
}


ScopedJObject::~ScopedJObject(){
    if(_thisObject){
        JNIEnv* env = get_java_jni_env();
        env->DeleteGlobalRef(_thisObject);
        _thisObject = nullptr;
    }
}