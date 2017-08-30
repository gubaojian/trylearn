//
// Created by furture on 2017/8/30.
//

#ifndef TSONPROJECT_TSONJS_H
#define TSONPROJECT_TSONJS_H

#include <JavaScriptCore/JavaScriptCore.h>
#include "tson.h"
#include <vector>
using namespace std;


void cpp_tson_push_js_value(JSContextRef ctx, JSValueRef value, tson_buffer *buffer);

JSValueRef cpp_tson_to_js_value(JSContextRef ctx, tson_buffer* buffer);


void cpp_tson_push_js_value_empty(JSContextRef ctx, JSValueRef value, tson_buffer *buffer);

#endif //TSONPROJECT_TSONJS_H
