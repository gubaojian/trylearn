package com.furture.litebrowser.jni;

import java.io.UnsupportedEncodingException;
import java.util.HashMap;
import java.util.Map;

/**
 * Created by furture on 2018/3/16.
 */

public class StringPool {

    private Map<Long,String> stringPool = new HashMap<>();

    public String getString(long ptr) throws UnsupportedEncodingException {
       String value =  stringPool.get(ptr);
       if(value != null){
           return value;
       }
       value = NativeUtils.getStringUtf8(ptr);
       stringPool.put(ptr, value);
       return  value;
    }

    public void reset(){
        stringPool.clear();
    }
}
