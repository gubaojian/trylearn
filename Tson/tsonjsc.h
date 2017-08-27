//
// Created by furture on 2017/8/26.
//

#ifndef HELLO_TSONJSC_H
#define HELLO_TSONJSC_H

//#if defined(__APPLE__)
#include <JavaScriptCore/JavaScriptCore.h>
//#endif
#include "tson.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * performance is not good, two many copy
 * */
void tson_push_js_value(JSContextRef ctx, JSValueRef value, tson_buffer *buffer);

JSValueRef tson_to_js_value(JSContextRef ctx, tson_buffer* buffer);


#ifdef __cplusplus
}
#endif


#endif //HELLO_TSONJSC_H
