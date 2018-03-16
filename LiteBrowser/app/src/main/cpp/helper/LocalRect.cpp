//
// Created by furture on 2018/3/15.
//

#include "LocalRect.h"
#include "JniHelper.h"


LocalRect::LocalRect(jobject rect):_thisRect(rect){
}

jfieldID LocalRect::getFieldId(const char* name, const char* sign){
    JNIEnv* env = get_java_jni_env();
    jclass  rectClass = env->GetObjectClass(_thisRect);
    jfieldID fieldId = env->GetFieldID(rectClass, name, sign);
    env->DeleteLocalRef(rectClass);
    return fieldId;
}

int LocalRect::left() {
    static jfieldID leftFieldId;
    if(!leftFieldId){
        leftFieldId = getFieldId("left", "I");
    }
    JNIEnv* env = get_java_jni_env();
    env->GetIntField(_thisRect, leftFieldId);
}

int LocalRect::top() {
    static jfieldID topFieldId;
    if(!topFieldId){
        topFieldId = getFieldId("top", "I");
    }
    JNIEnv* env = get_java_jni_env();
    env->GetIntField(_thisRect, topFieldId);
}



int LocalRect::right() {
    static jfieldID rightFieldId;
    if(!rightFieldId){
        rightFieldId = getFieldId("right", "I");
    }
    JNIEnv* env = get_java_jni_env();
    env->GetIntField(_thisRect, rightFieldId);
}


int LocalRect::bottom() {
    static jfieldID bottomFieldId;
    if(!bottomFieldId){
        bottomFieldId = getFieldId("right", "I");
    }
    JNIEnv* env = get_java_jni_env();
    env->GetIntField(_thisRect, bottomFieldId);
}

LocalRect::~LocalRect() {
    if(_thisRect){
        JNIEnv* env = get_java_jni_env();
        env->DeleteLocalRef(_thisRect);
        _thisRect = nullptr;
    }
}