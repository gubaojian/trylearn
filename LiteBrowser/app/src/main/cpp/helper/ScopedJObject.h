//
// Created by furture on 2018/3/12.
//

#ifndef LITEBROWSER_SCOPEDJOBJECT_H
#define LITEBROWSER_SCOPEDJOBJECT_H

#include <jni.h>


class ScopedJObject {
public:
    ScopedJObject(jobject thisObject);
    ~ScopedJObject();
    jobject getObject(){ return _thisObject;};

private:
    jobject _thisObject;
};


#endif //LITEBROWSER_SCOPEDJOBJECT_H
