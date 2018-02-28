package com.furture.myapplication;

import android.util.Log;

/**
 * Created by furture on 2018/2/27.
 */

public class NativeIpc {
    static {
        System.loadLibrary("ipctest");
        Log.e("weex", "ipc load ipc library nativeipc");
    }

    public static native void ipc(byte[] bts);

    public static native void exit();
}
