//
// Created by furture on 2018/3/15.
//

#ifndef LITEBROWSER_LOCALRECT_H
#define LITEBROWSER_LOCALRECT_H


#include <jni.h>

class LocalRect {

public:
    LocalRect(jobject rect);
    ~LocalRect();
    int left();
    int top();
    int right();
    int bottom();
private:
    jobject _thisRect;
    jfieldID getFieldId(const char* name, const char* sign);
};


#endif //LITEBROWSER_LOCALRECT_H
