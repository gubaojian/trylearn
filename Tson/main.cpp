#include <iostream>
#include <fstream>

using  namespace std;

#include "tsonjsc.h"
#include "data.h"
#include "tsonjs.h"
#include <vector>

static void  testTsonDouble();
static void testTsonWriteFile();
static void testTsonParse();
static  void testTsonJsc();
static void testVector();
char* JSValueUtf8(JSContextRef ctx, JSValueRef value);
char* JSStringUtf8(JSContextRef ctx, JSStringRef string);

static double now_ms(void) {
    struct timespec res;
    clock_gettime(CLOCK_REALTIME, &res);
    return 1000.0 * res.tv_sec + (double) res.tv_nsec / 1e6;
}


int main() {
    std::cout << "Tson, Test Case" << std::endl;
    //testTsonDouble();
    //testTsonWriteFile();
    //testTsonParse();
    //testVector();
    testTsonJsc();

    return 0;
}

static void testTsonParse(){
    tson_buffer* buffer = tson_buffer_new();
    tson_push_double(buffer, 20.3356);
    tson_push_int(buffer, 12345);
    buffer->position = 0;
    double d = tson_next_double(buffer);
    if(d == 20.3356){
        printf("passed double\n");
    }else{
        printf("failed double\n");
    }
    int32_t  i = tson_next_int(buffer);
    if( i  == 12345){
        printf("pass int %d\n",  i);
    }else{
        printf("failed int\n");
    }

    if(buffer->position == 12){
        printf("passed position\n");
    }
    buffer->position = 0;
    tson_push_int(buffer, -123456);
    printf("pass varint %d\n",  buffer->position);
    buffer->position = 0;
    int32_t  varint = tson_next_int(buffer);
    if(varint == -123456){
        printf("pass varint %d\n",  varint);
    }else{
        printf("failed varint %d\n", varint);
    }

    tson_buffer_free(buffer);
}
static void testVector(){
    vector<int> vector;
    vector.push_back(10);
    vector.back()++;
    printf("vector back %d", vector.back());
}
static void testTsonJsc(){
    tson_buffer* buffer = tson_buffer_new();
    JSGlobalContextRef globalContext = JSGlobalContextCreate(NULL);
    JSObjectRef  objectRef = JSContextGetGlobalObject(globalContext);

    char const* fileName= "./../data.json";
    char* json = readJSONFile(fileName);


    JSStringRef  string = JSStringCreateWithUTF8CString(json);
    long parse  = now_ms();
    JSValueRef valueRef = JSValueMakeFromJSONString(globalContext, string);
    if(valueRef == NULL){
        printf("parse error\n");
    }
    printf("parse used %f\n", (now_ms() - parse));
    double start = now_ms();
    tson_push_js_value(globalContext, valueRef, buffer);
    printf("tson used %f %d \n", (now_ms() - start), buffer->position);

    start = now_ms();
    //cpp_tson_push_js_value(globalContext, valueRef, buffer);
    printf("cpp tson used %f  %d\n", (now_ms() - start), buffer->position);



    JSStringRef JSON = JSStringCreateWithUTF8CString("JSON");
    JSObjectRef JSONRef = JSValueToObject(globalContext,JSObjectGetProperty(globalContext, objectRef, JSON, 0), 0);
    JSStringRelease(JSON);
    JSStringRef  STRINGIFY = JSStringCreateWithUTF8CString("stringify");
    JSObjectRef function = JSValueToObject(globalContext, JSObjectGetProperty(globalContext, JSONRef, STRINGIFY, 0), 0);
    JSStringRelease(STRINGIFY);
    JSValueRef arguments[1];
    arguments[0] = valueRef;
    printf("value ref %s\n", JSValueUtf8(globalContext, valueRef));
    start = now_ms();
    JSValueRef  jsonStringRef = JSObjectCallAsFunction(globalContext, function, NULL, 1, arguments,  0);
    printf("json stringfy used %f \n", (now_ms() - start));

   printf("json %s\n", JSValueUtf8(globalContext,jsonStringRef));

    start = now_ms();
    buffer->position = 0;
    JSValueRef  tsonValueRef = tson_to_js_value(globalContext, buffer);
    printf("tson parse used %f \n",  (now_ms() - start));
    arguments[0] = tsonValueRef;
    JSValueRef  tsonStringRef = JSObjectCallAsFunction(globalContext, function, NULL, 1, arguments,  0);
    printf("tson %s\n", JSValueUtf8(globalContext, tsonStringRef));


    JSGlobalContextRelease(globalContext);
    free(json);
    tson_buffer_free(buffer);
    //JSValueMakeFromJSONString()
}


char* JSValueUtf8(JSContextRef ctx, JSValueRef value){
    JSStringRef  string = JSValueToStringCopy(ctx, value, 0);
    int length = JSStringGetMaximumUTF8CStringSize(string);
    char* buffer = static_cast<char *>(malloc(sizeof(char) * length));
    JSStringGetUTF8CString(string, buffer, length);
    JSStringRelease(string);
    return  buffer;
}

char* JSStringUtf8(JSContextRef ctx, JSStringRef string){
    int length = JSStringGetMaximumUTF8CStringSize(string);
    char* buffer = static_cast<char *>(malloc(sizeof(char) * length));
    JSStringGetUTF8CString(string, buffer, length);
    return  buffer;
}

static void testTsonWriteFile(){
    tson_buffer* buffer = tson_buffer_new();
    double d = 210.33576;
    tson_push_double(buffer, d);
    tson_push_int(buffer, 12345);
    tson_push_int(buffer, -1);
    tson_push_int(buffer, INT32_MAX);
    tson_push_int(buffer, INT32_MIN);
    tson_push_uint(buffer, 1);
    tson_push_uint(buffer, INT32_MAX);
    ofstream os;
    os.open("data.dat", ios_base::binary);
    os.write((char*)buffer->data, buffer->position);
    os.close();
    buffer->position = 0;
    double gd = tson_next_double(buffer);
    if(gd == d){
        printf("passed double \n");
    }else{
        printf("failed double \n");
    }
    int32_t gi  = tson_next_int(buffer);
    if(gi == 12345){
        printf("pass int %d\n", gi);
    }else{
        printf("failed int %d\n", gi);
    }


    tson_buffer_free(buffer);

}

static void  testTsonDouble(){
    tson_buffer* buffer = tson_buffer_new();

    int count = 1000;
    double start = now_ms();
    for(int i=0; i<count; i++){
        tson_push_double(buffer, (double) i);
    }
    printf("used %f", (now_ms() - start));
    buffer->position = 0;
    for(int i=0; i<count; i++){
        double  num = tson_next_double(buffer);
        if(num  != i){
            printf("failed parse double int %f  %d\n ", num , i);
        }
    }
    tson_buffer_free(buffer);
}