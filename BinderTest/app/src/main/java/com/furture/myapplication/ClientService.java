package com.furture.myapplication;

import android.app.Service;
import android.content.Intent;
import android.os.Binder;
import android.os.IBinder;
import android.os.RemoteException;
import android.support.annotation.Nullable;
import android.util.Log;

/**
 * Created by furture on 2018/2/27.
 */

public class ClientService extends Service {


    public class HelloBinder extends Binder{

        public void sayIpc(byte[] bts){

        }
    }

    private Binder stub = new BinderIpc.Stub() {

        @Override
        public void ipcBytes(byte[] bts) throws RemoteException {
            Log.e("weex", "weex current thread "
                    + Thread.currentThread().getName()
            +  android.os.Process.myPid());
        }

        @Override
        public void ipcString(String string) throws RemoteException {

        }
    };

    public ClientService() {
    }


    @Nullable
    @Override
    public IBinder onBind(Intent intent) {
        return stub;
    }
}
