package com.furture.litebrowser.jni;

import android.util.Log;

import java.io.UnsupportedEncodingException;

/**
 * Created by furture on 2018/3/15.
 */

public class NativeUtils {


    /**
     * update native size pointer width and height
     * */
    public static native  void setSize(long ptr, int width, int height);


    /**
     * get String from native UTF-8
     * */
    public static String getStringUtf8(long ptr) throws UnsupportedEncodingException {
            byte[] bts = getStringBytes(ptr);
            return new String(bts, "UTF-8");
    }

    private static native byte[] getStringBytes(long ptr);
}
