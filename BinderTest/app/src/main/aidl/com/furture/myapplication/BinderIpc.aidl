// BinderIpcService.aidl
package com.furture.myapplication;

// Declare any non-default types here with import statements

interface BinderIpc {
    /**
     * Demonstrates some basic types that you can use as parameters
     * and return values in AIDL.
     */
    oneway void ipcBytes(in byte[] bts);

    void ipcString(String string);
}
