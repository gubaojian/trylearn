//
// Created by furture on 2017/8/26.
//
#include "tsonjsc.h"


void tson_push_js_value(JSContextRef ctx, JSValueRef value, tson_buffer *buffer){
    JSType type = JSValueGetType(ctx, value);
    switch (type) {
        case kJSTypeString: {
            JSStringRef string = JSValueToStringCopy(ctx, value, 0);
            int length = JSStringGetMaximumUTF8CStringSize(string);
            char* utf8 = malloc(sizeof(char)*length);
            int size = JSStringGetUTF8CString(string, utf8, length);
            tson_push_type_string(buffer, utf8, size);
            free(utf8);
            JSStringRelease(string);
        }
            return;
        case kJSTypeObject: {
            JSObjectRef objectRef = JSValueToObject(ctx, value, 0);
            if (JSValueIsArray(ctx, value)) {
                JSStringRef lengthString = JSStringCreateWithUTF8CString("length");
                unsigned int length = (unsigned int) JSValueToNumber(ctx,
                                                                     JSObjectGetProperty(ctx, objectRef, lengthString,
                                                                                         0), 0);
                JSStringRelease(lengthString);
                tson_push_type_array(buffer, length);
                for (int i = 0; i < length; i++) {
                    tson_push_js_value(ctx, JSObjectGetPropertyAtIndex(ctx, objectRef, i, 0), buffer);
                }
            } else {
                JSPropertyNameArrayRef propertyNameArray = JSObjectCopyPropertyNames(ctx, objectRef);
                int length = JSPropertyNameArrayGetCount(propertyNameArray);
                tson_push_type_map(buffer, length);
                for (int i = 0; i < length; i++) {
                    JSStringRef propertyName = JSPropertyNameArrayGetNameAtIndex(propertyNameArray, i);
                    int propertyLength = JSStringGetMaximumUTF8CStringSize(propertyName);
                    char* utf8 = malloc(propertyLength* sizeof(char));
                    int size = JSStringGetUTF8CString(propertyName,  utf8, propertyLength);
                    tson_push_uint(buffer, size);
                    tson_push_bytes(buffer, utf8, size);
                    free(utf8);
                    tson_push_js_value(ctx, JSObjectGetProperty(ctx, objectRef, propertyName, 0), buffer);
                }
                JSPropertyNameArrayRelease(propertyNameArray);
            }
        }
            return;
        case kJSTypeNumber: {
            double num = JSValueToNumber(ctx, value, NULL);
            tson_push_type_double(buffer, num);
        }
            return;
        case kJSTypeBoolean: {
            bool isTrue = JSValueToBoolean(ctx, value);
            tson_push_type_boolean(buffer, isTrue);
        }
            return;
        case kJSTypeNull:
        case kJSTypeUndefined: {
            tson_push_type_null(buffer);
        }
            return;
        default:
            return;
    }
}

JSValueRef tson_to_js_value(JSContextRef ctx, tson_buffer* buffer){
    uint8_t  type = tson_next_type(buffer);
    JSValueRef  ref = NULL;
    switch (type){
        case TSON_STRING_TYPE:{
                int length = tson_next_uint(buffer);
                char* utf8 = (char*)tson_next_bts(buffer, length);
                JSStringRef  string = JSStringCreateWithUTF8CString(utf8);
                ref = JSValueMakeString(ctx, string);
                JSStringRelease(string);
            }
            break;
        case TSON_ARRAY_TYPE:{
            int length = tson_next_uint(buffer);
            JSValueRef values[length];
            for(int i=0; i<length; i++){
                values[i] = tson_to_js_value(ctx, buffer);
            }
            ref = JSObjectMakeArray(ctx, length, values, NULL);
        }
            break;
        case TSON_MAP_TYPE:{
            int length = tson_next_uint(buffer);
            JSObjectRef object  = JSObjectMake(ctx, NULL, NULL);
            for(int i=0; i<length; i++){
                int propertyLength = tson_next_uint(buffer);
                char* utf8 = (char*)tson_next_bts(buffer, propertyLength);
                JSStringRef  propertyName = JSStringCreateWithUTF8CString(utf8);
                JSObjectSetProperty(ctx, object, propertyName, tson_to_js_value(ctx, buffer), kJSPropertyAttributeNone, NULL);
                JSStringRelease(propertyName);
            }
            ref = object;
        }
            break;
        case TSON_NUMBER_DOUBLE_TYPE:{
            double  num = tson_next_double(buffer);
            ref = JSValueMakeNumber(ctx, num);
        }
            break;
        case TSON_BOOLEAN_TYPE:{
            bool value = (tson_next_byte(buffer) != 0);
            ref = JSValueMakeBoolean(ctx, value);
        }
            break;
        case TSON_NUMBER_INT_TYPE:{
            double  num = tson_next_int(buffer);
            ref = JSValueMakeNumber(ctx, num);
        }
            break;
        default:
            break;
    }
    return ref;
}