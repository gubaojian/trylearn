//
// Created by furture on 2018/3/12.
//

#ifndef LITEBROWSER_LITEBROWSERCOMMON_H
#define LITEBROWSER_LITEBROWSERCOMMON_H

#include <jni.h>

typedef struct MobileScreen{
    int width;
    int height;
    float density;
    int densityDpi;
    int  defaultFontSize;
} MobileScreen;
typedef struct LiteBrowserJava{
    JavaVM*  jvm;
    MobileScreen screen;
    jclass  browserClass;
    jmethodID createFontFaceMethodId;
}LiteBrowserJava;



#define convert_to_ptr(ctx)    ((jlong)((intptr_t)ctx))



#endif //LITEBROWSER_LITEBROWSERCOMMON_H
