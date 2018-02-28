package com.furture.myapplication;

/**
 * Created by furture on 2018/2/27.
 */

/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Original file: /Users/furture/code/trylearn/BinderTest/app/src/main/aidl/com/furture/myapplication/BinderIpc.aidl
 */

public interface BinderIpcCustom extends android.os.IInterface
{
    /** Local-side IPC implementation stub class. */
    public static abstract class Stub extends android.os.Binder implements com.furture.myapplication.BinderIpcCustom
    {
        private static final java.lang.String DESCRIPTOR = "com.furture.myapplication.BinderIpcCustom";
        /** Construct the stub at attach it to the interface. */
        public Stub()
        {
            this.attachInterface(this, DESCRIPTOR);
        }
        /**
         * Cast an IBinder object into an com.furture.myapplication.BinderIpc interface,
         * generating a proxy if needed.
         */
        public static com.furture.myapplication.BinderIpcCustom asInterface(android.os.IBinder obj)
        {
            if ((obj==null)) {
                return null;
            }
            android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
            if (((iin!=null)&&(iin instanceof com.furture.myapplication.BinderIpcCustom))) {
                return ((com.furture.myapplication.BinderIpcCustom)iin);
            }
            return new com.furture.myapplication.BinderIpcCustom.Stub.Proxy(obj);
        }
        @Override public android.os.IBinder asBinder()
        {
            return this;
        }
        @Override public boolean onTransact(int code, android.os.Parcel data, android.os.Parcel reply, int flags) throws android.os.RemoteException
        {
            switch (code)
            {
                case INTERFACE_TRANSACTION:
                {
                    reply.writeString(DESCRIPTOR);
                    return true;
                }
                case TRANSACTION_ipcBytes:
                {
                    data.enforceInterface(DESCRIPTOR);
                    //byte[] _arg0;
                    //_arg0 = data.createByteArray();
                    //this.ipcBytes(_arg0);
                    reply.writeNoException();
                    return true;
                }
                case TRANSACTION_ipcString:
                {
                    data.enforceInterface(DESCRIPTOR);
                    //java.lang.String _arg0;
                    //_arg0 = data.readString();
                    //this.ipcString(_arg0);
                    reply.writeNoException();
                    return true;
                }
            }
            return super.onTransact(code, data, reply, flags);
        }
        private static class Proxy implements com.furture.myapplication.BinderIpcCustom
        {
            private android.os.IBinder mRemote;
            Proxy(android.os.IBinder remote)
            {
                mRemote = remote;
            }
            @Override public android.os.IBinder asBinder()
            {
                return mRemote;
            }
            public java.lang.String getInterfaceDescriptor()
            {
                return DESCRIPTOR;
            }
            /**
             * Demonstrates some basic types that you can use as parameters
             * and return values in AIDL.
             */
            @Override public void ipcBytes(byte[] bts) throws android.os.RemoteException
            {
                android.os.Parcel _data = android.os.Parcel.obtain();
                android.os.Parcel _reply = android.os.Parcel.obtain();
                try {
                    _data.writeInterfaceToken(DESCRIPTOR);
                    _data.writeByteArray(bts);
                    for(int i=0; i<1000; i++) {
                        mRemote.transact(Stub.TRANSACTION_ipcBytes, _data, _reply, 0);
                        _reply.readException();
                    }
                }
                finally {
                    _reply.recycle();
                    _data.recycle();
                }
            }
            @Override public void ipcString(java.lang.String string) throws android.os.RemoteException
            {
                android.os.Parcel _data = android.os.Parcel.obtain();
                android.os.Parcel _reply = android.os.Parcel.obtain();
                try {
                    _data.writeInterfaceToken(DESCRIPTOR);
                    _data.writeString(string);
                    mRemote.transact(Stub.TRANSACTION_ipcString, _data, _reply, 0);
                    _reply.readException();
                }
                finally {
                    _reply.recycle();
                    _data.recycle();
                }
            }
        }
        static final int TRANSACTION_ipcBytes = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
        static final int TRANSACTION_ipcString = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    }
    /**
     * Demonstrates some basic types that you can use as parameters
     * and return values in AIDL.
     */
    public void ipcBytes(byte[] bts) throws android.os.RemoteException;
    public void ipcString(java.lang.String string) throws android.os.RemoteException;
}
