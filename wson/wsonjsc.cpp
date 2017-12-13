//
//  wsonjsc.cpp
//  JavaScriptCore
//
//  Created by furture on 2017/8/30.
//
//

#include "wsonjsc.h"
#include "ObjectConstructor.h"
#include "JSONObject.h"
#include <wtf/Vector.h>
#include <wtf/HashMap.h>


#include <android/log.h>
#define TAG "ICUCompatible"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

using namespace JSC;

/**
 * max deep, like JSONObject.cpp's maximumFilterRecursion default 40000
 */
#define WSON_MAX_DEEP  40000
#define WSON_SYSTEM_IDENTIFIER_CACHE_COUNT (1024*4)
#define WSON_LOCAL_IDENTIFIER_CACHE_COUNT 64

namespace wson {
    struct IdentifierCache{
        Identifier identifer = Identifier::EmptyIdentifier;
        size_t length = 0;
        uint32_t key = 0;
        const UChar* utf16 = NULL;
    };

    static  IdentifierCache* systemIdentifyCache = nullptr;
    static  VM* systemIdentifyCacheVM = nullptr;
    void wson_push_js_value(ExecState* exec, JSValue val, wson_buffer* buffer, Vector<JSObject*, 16>& objectStack);
    JSValue wson_to_js_value(ExecState* state, wson_buffer* buffer, IdentifierCache* localIdentifiers);
    inline void wson_push_js_string(ExecState* exec,  JSValue val, wson_buffer* buffer);
    inline void wson_push_js_identifier(Identifier val, wson_buffer* buffer);




    wson_buffer* toWson(ExecState* exec, JSValue val){
         LOGE("weex wson pre %s", JSONStringify(exec, val, 0).utf8().data());
        wson_buffer* buffer = wson_buffer_new();
        LocalScope localScope(exec->vm());
        Vector<JSObject*, 16> objectStack;
        wson_push_js_value(exec, val, buffer, objectStack);
        int position = buffer->position;
        buffer->position = 0;
        buffer->length = position;
        JSValue back = toJSValue(exec, buffer);
        LOGE("weex wson back %s", JSONStringify(exec, back, 0).utf8().data());

        buffer->position = position; 
        return buffer;
    }

    /**
     * if has system cache, local cache size 64, if not, local cache size 128
     */
    JSValue toJSValue(ExecState* exec, wson_buffer* buffer){
        VM& vm =exec->vm();
        LocalScope scope(vm);
        if(systemIdentifyCacheVM && systemIdentifyCacheVM == &vm){
            IdentifierCache localIdentifiers[WSON_LOCAL_IDENTIFIER_CACHE_COUNT];
            return wson_to_js_value(exec, buffer, localIdentifiers);
        }
        IdentifierCache localIdentifiers[WSON_LOCAL_IDENTIFIER_CACHE_COUNT*2];
        JSValue value =  wson_to_js_value(exec, buffer, localIdentifiers);
        LOGE("weex wson toJSValue %s", JSONStringify(exec,value, 0).utf8().data());
        LOGE("weex wson toJSValue %s", JSONStringify(exec,value, 0).utf8().data());
        
        return JSONParse(exec, JSONStringify(exec,value, 0));
    }

     JSValue toJSValue(ExecState* state, void* data, int length){
         wson_buffer* buffer = wson_buffer_from(data, length);
         JSValue ret = toJSValue(state, buffer);
         buffer->data = nullptr;
         wson_buffer_free(buffer);
         return ret;
     }


    void init(VM* vm){
        if(systemIdentifyCache){
            destory();
        }
        if(systemIdentifyCacheVM){
            systemIdentifyCacheVM = nullptr;
        }
        systemIdentifyCache = new IdentifierCache[WSON_SYSTEM_IDENTIFIER_CACHE_COUNT];
        for(int i=0; i<WSON_SYSTEM_IDENTIFIER_CACHE_COUNT; i++){
            systemIdentifyCache[i].identifer = vm->propertyNames->nullIdentifier;
            systemIdentifyCache[i].length = 0;
            systemIdentifyCache[i].utf16 = NULL;
            systemIdentifyCache[i].key = 0;
        }
        systemIdentifyCacheVM  = vm;
    }


    void destory(){
        if(systemIdentifyCache){
            for(int i=0; i<WSON_SYSTEM_IDENTIFIER_CACHE_COUNT; i++){
                if(systemIdentifyCache[i].length > 0){
                    if(systemIdentifyCache[i].utf16){
                        //free((void*)(systemIdentifyCache[i].utf16));
                        systemIdentifyCache[i].utf16 = NULL;
                    }
                    systemIdentifyCache[i].key = 0;
                    systemIdentifyCache[i].length = 0;
                }
            }
            delete[] systemIdentifyCache; 
            systemIdentifyCache = nullptr;
        }
        if(systemIdentifyCacheVM){
            systemIdentifyCacheVM = nullptr;
        }
    }



    /**
     * check is circle reference and  max deep
     */
    inline bool check_js_deep_and_circle_reference(JSObject* object, Vector<JSObject*, 16>& objectStack){
        for (unsigned i = 0; i < objectStack.size(); i++) {
            if (objectStack.at(i) == object) {
                return true;
            }
        }
        if(objectStack.size() > WSON_MAX_DEEP){
            return true;
        }
        return false;
    }

    /**
      * djb2-hash-function
      */
    inline uint32_t hash(const UChar* utf16, const int length){
        uint32_t  hash = 5381;
        for(int i=0;i<length; i++){
            hash = ((hash << 5) + hash) + utf16[i];
        }
        return hash;
    }

    inline uint32_t hash2(const UChar* utf16, const int length){
        uint32_t  hash = 5381;
        hash = ((hash << 5) + hash) + utf16[0];
        if(length > 1){
            hash = ((hash << 5) + hash) + utf16[length-1];
        }
        return hash;
    }

    /**
     * most of json identifer is repeat, cache can improve performance
     */
    inline Identifier makeIdentifer(VM* vm, IdentifierCache* localIdentifiers, const UChar* utf16, size_t length){
        if(length <= 0){
           return vm->propertyNames->emptyIdentifier;
        }
        if (utf16[0] <= 0 || utf16[0] >= 127 || length > 32){//only cache short identifier
            UChar* destination;
            String string = String::createUninitialized(length, destination);
            memcpy((void*)destination, (void*)utf16, length*sizeof(UChar));
            return  Identifier::fromString(vm, string);
        }
        uint32_t key = 0;
        uint32_t systemIdentifyCacheIndex = 0;
        IdentifierCache cache;
        bool saveGlobal = false;
        uint32_t localIndex = 0;
        if(systemIdentifyCacheVM && systemIdentifyCacheVM == vm){
            key = hash(utf16, length);
            systemIdentifyCacheIndex = (WSON_SYSTEM_IDENTIFIER_CACHE_COUNT - 1)&key;
            localIndex = (WSON_LOCAL_IDENTIFIER_CACHE_COUNT - 1) & key;
            if(systemIdentifyCache != nullptr){
                cache = systemIdentifyCache[systemIdentifyCacheIndex];
                if(cache.length == length
                   && cache.key == key
                   && memcmp((void*)cache.utf16, (void*)utf16, length*sizeof(UChar)) == 0
                   && !cache.identifer.isNull()){
                    return cache.identifer;
                }
                if(!cache.utf16 && cache.length == 0){
                    saveGlobal = true;
                }
            }
        }else{
            key = hash2(utf16, length);
            localIndex = (WSON_LOCAL_IDENTIFIER_CACHE_COUNT*2 - 1) & key;
        }

        cache = localIdentifiers[localIndex];
        if(cache.length == length
           && cache.key == key
           && memcmp((void*)cache.utf16, (void*)utf16, length) == 0
           && !cache.identifer.isNull()){
            return cache.identifer;
        }

        UChar* destination;
        String string = String::createUninitialized(length, destination);
        memcpy((void*)destination, (void*)utf16, length*sizeof(UChar));
        Identifier identifier = Identifier::fromString(vm, string);
        cache.identifer = identifier;
        cache.length = length;
        cache.key = key;
        if(saveGlobal && length <= 32){
            cache.utf16 = destination;
            systemIdentifyCache[systemIdentifyCacheIndex] = cache;
        }else{
            cache.utf16 = destination;
            localIdentifiers[localIndex] = cache;
        }
        return identifier;
    }

    JSValue wson_to_js_value(ExecState* exec, wson_buffer* buffer,  IdentifierCache* localIdentifiers){
        uint8_t  type = wson_next_type(buffer);
        switch (type) {
            case WSON_STRING_TYPE:{
                    uint32_t length = wson_next_uint(buffer);
                    UChar* destination;
                    String s = String::createUninitialized(length/sizeof(UChar), destination);
                    void* src = wson_next_bts(buffer, length);
                    memcpy(destination, src, length);
                    return jsString(exec, s);
                }
                break;
            case WSON_ARRAY_TYPE:{
                    uint32_t length = wson_next_uint(buffer);
                    JSArray* array = constructEmptyArray(exec, 0);
                    for(uint32_t i=0; i<length; i++){
                        if(wson_has_next(buffer)){
                            array->putDirectIndex(exec, i, wson_to_js_value(exec, buffer, localIdentifiers));
                        }else{
                            break;
                        }
                    }
                   return array;
                }
                break;
            case WSON_MAP_TYPE:{
                  uint32_t length = wson_next_uint(buffer);
                  JSObject* object = constructEmptyObject(exec);
                  VM& vm = exec->vm();
                  for(uint32_t i=0; i<length; i++){
                      if(wson_has_next(buffer)){
                          int propertyLength = wson_next_uint(buffer);
                          const UChar* data = (const UChar*)wson_next_bts(buffer, propertyLength);
                          PropertyName name = makeIdentifer(&vm, localIdentifiers, data, propertyLength/sizeof(UChar));
                          if (std::optional<uint32_t> index = parseIndex(name)){
                              object->putDirectIndex(exec, index.value(), wson_to_js_value(exec, buffer, localIdentifiers));
                          }else{
                              object->putDirect(vm, name, wson_to_js_value(exec, buffer, localIdentifiers));
                          }
                      }else{
                          break;
                      }
                   }
                   return object;
                }
                break;   
           case WSON_NUMBER_INT_TYPE:{
                    int32_t  num =  wson_next_int(buffer);
                    return  jsNumber(num);
                } 
                break;  
            case WSON_BOOLEAN_TYPE_TRUE:{
                  return jsBoolean(true);
               }
               break;
            case WSON_BOOLEAN_TYPE_FALSE:{
                return jsBoolean(false);
                 }
                 break;   
            case WSON_NUMBER_DOUBLE_TYPE:{
                 double  num = wson_next_double(buffer);
                 return  jsNumber(num);
                }
                break;
            case WSON_NULL_TYPE:{
                 return  jsUndefined();
                }
                break;    
            default:
                LOGE("weex weex wson err  wson_to_js_value  unhandled type %d buffer position  %d length %d", type, buffer->position, buffer->length);
                break;
        }
        return jsUndefined();
    }

    void wson_push_js_value(ExecState* exec, JSValue val, wson_buffer* buffer, Vector<JSObject*, 16>& objectStack){
        if(val.isNull() || val.isUndefined() || val.isEmpty()){
            wson_push_type_null(buffer);
            return;
        }

        if(val.isString()){
            wson_push_js_string(exec, val, buffer);
            return;
        }

        if(val.isNumber()){
            if(val.isInt32()){
                wson_push_type_int(buffer, val.asInt32());
            }else{
                double number = val.asNumber();
                wson_push_type_double(buffer, number);
            }
            return;
        }

        if(isJSArray(val)){
            JSArray* array = asArray(val);
            if(check_js_deep_and_circle_reference(array, objectStack)){
                wson_push_type_null(buffer);
                return;
            }
            uint32_t length = array->length();
            wson_push_type_array(buffer, length);
            objectStack.append(array);
            for(uint32_t index=0; index<length; index++){
                JSValue ele = array->getIndex(exec, index);
                if(ele.isFunction()){
                     JSCell* cell = ele.asCell();
                     VM& vm = exec->vm();
                     MarkedArgumentBuffer args;
                     if(index <= 9){
                         args.append(vm.smallStrings.singleCharacterString(index + '0'));
                     }else{
                         args.append(jsNontrivialString(&vm, vm.numericStrings.add(index)));
                     }
                    args.append(ele);
                    CallData callData;
                    CallType callType = cell->methodTable()->getCallData(cell, callData);
                    ele = call(exec, ele, callType, callData, array, args);
                }
                wson_push_js_value(exec, ele, buffer, objectStack);
            }
            objectStack.removeLast();
            return;
        }

        if(val.isObject() && !val.isFunction()){
            JSObject* object = asObject(val);
            VM& vm = exec->vm();

            if (object->inherits(vm, StringObject::info())){
                wson_push_js_string(exec, object->toString(exec), buffer);
                return;
            }

            if (object->inherits(vm, NumberObject::info())){
                double number = object->toNumber(exec);
                wson_push_type_double(buffer, number);
                return;
            }

            if (object->inherits(vm, BooleanObject::info())){
                JSValue boolVal =  object->toPrimitive(exec);
                if(boolVal.isTrue()){
                    wson_push_type_boolean(buffer, 1);
                }else{
                    wson_push_type_boolean(buffer, 0);
                }
                return;
            }

            if(check_js_deep_and_circle_reference(object, objectStack)){
                wson_push_type_null(buffer);
                return;
            }
            PropertyNameArray objectPropertyNames(exec, PropertyNameMode::Strings);
            const MethodTable* methodTable = object->methodTable();
            methodTable->getOwnPropertyNames(object, exec, objectPropertyNames, EnumerationMode());
            PropertySlot slot(object, PropertySlot::InternalMethodType::Get);
            uint32_t size = objectPropertyNames.size();

            /** first check skip null or function */
            uint32_t nullOrFunctionSize  = 0;
            for(uint32_t i=0; i<size; i++){
                 Identifier& propertyName = objectPropertyNames[i];
                 if(methodTable->getOwnPropertySlot(object, exec, propertyName, slot)){
                     JSValue propertyValue = slot.getValue(exec, propertyName);
                     if(propertyValue.isUndefinedOrNull() || propertyValue.isFunction()){
                         nullOrFunctionSize++;
                     }
                 }else{
                      nullOrFunctionSize++;
                 }
            }
            wson_push_type_map(buffer, size - nullOrFunctionSize);
            objectStack.append(object);
            for(uint32_t i=0; i<size; i++){
                 Identifier& propertyName = objectPropertyNames[i];
                 wson_push_js_identifier(propertyName , buffer);
                 if(methodTable->getOwnPropertySlot(object, exec, propertyName, slot)){
                     JSValue propertyValue = slot.getValue(exec, propertyName);
                     if(propertyValue.isFunction()){
                           LOGE("weex wson err properyName %s", propertyName.string().utf8().data());
                           for(int j=0; j<objectStack.size(); j++){
                                LOGE("weex wson err value  properyName type is not handled, root json value %s", JSONStringify(exec, objectStack[j], 0).utf8().data());
                           } 
                           JSCell* cell = propertyValue.asCell();
                           MarkedArgumentBuffer args;
                           args.append(jsString(exec, propertyName.string()));
                           args.append(propertyValue);
                           CallData callData;
                           CallType callType = cell->methodTable()->getCallData(cell, callData);
                           propertyValue = call(exec, propertyValue, callType, callData, object, args);
                           LOGE("weex wson err value  properyNametype, treat as %d %d %s", propertyValue.isUndefinedOrNull(),
                           propertyValue.isFunction(), JSONStringify(exec, propertyValue, 0).utf8().data());
                     }else{
                          LOGE("weex wson ok properyName %s", propertyName.string().utf8().data());
                     }
                     wson_push_js_value(exec, propertyValue, buffer,objectStack);
                 }else{
                    wson_push_type_null(buffer);
                 }
            }
            objectStack.removeLast();
            return;
        }
        
        if(val.isBoolean()){
            if(val.isTrue()){
                 wson_push_type_boolean(buffer, 1);
            }else{
                wson_push_type_boolean(buffer, 0);
            }
            return;
        }
        if(val.isFunction()){
            LOGE("weex wson err value type  unhandled function");
        }

#ifdef LOGE
        LOGE("weex wson err value type is not handled, treat as null, json value %s %d %d ", JSONStringify(exec, val, 0).utf8().data(), val.isFunction(), val.tag());
        for(int i=0; i<objectStack.size(); i++){
            LOGE("weex wson err value type is not handled, treat as null, root json value %s", JSONStringify(exec, objectStack[i], 0).utf8().data());
        }
#endif
        wson_push_type_null(buffer);
    }

    inline void wson_push_js_string(ExecState* exec,  JSValue val, wson_buffer* buffer){
        String s = val.toWTFString(exec);
        int length = s.length();
        if (s.is8Bit()) {
            // Convert latin1 chars to unicode.
            Vector<UChar> jchars(s.length());
            for (unsigned i = 0; i < length; i++) {
                jchars[i] = s.at(i);
            }
            wson_push_type(buffer, WSON_STRING_TYPE);  
            wson_push_uint(buffer, length*sizeof(UChar));
            wson_push_bytes(buffer, jchars.data(), s.length()*sizeof(UChar));
           // LOGE("json log");
       } else {
           wson_push_type(buffer, WSON_STRING_TYPE);  
           wson_push_uint(buffer, length*sizeof(UChar));
           wson_push_bytes(buffer, s.characters16(), s.length()*sizeof(UChar));
        }

    }

    inline void wson_push_js_identifier(Identifier val, wson_buffer* buffer){
         String s = val.string();
          int length = s.length();
         if (s.is8Bit()) {
            // Convert latin1 chars to unicode.
            Vector<UChar> jchars(s.length());
            for (unsigned i = 0; i < length; i++) {
                jchars[i] = s.at(i);
            }
            wson_push_uint(buffer, length*sizeof(UChar));
            wson_push_bytes(buffer, jchars.data(), s.length()*sizeof(UChar));
        } else { 
           wson_push_uint(buffer, length*sizeof(UChar));
           wson_push_bytes(buffer, s.characters16(), s.length()*sizeof(UChar));
        }
    }
}
