package com.furture.myapplication;

import android.app.IntentService;
import android.app.Service;
import android.content.Intent;
import android.os.Binder;
import android.os.IBinder;
import android.os.RemoteException;
import android.support.annotation.Nullable;

/**
 * Created by furture on 2018/2/27.
 */

public class BinderService extends Service {


    public class HelloBinder extends Binder{

        public void sayIpc(byte[] bts){

        }
    }

    private Binder stub = new BinderIpcCustom.Stub() {

        @Override
        public void ipcBytes(byte[] bts) throws RemoteException {

        }

        @Override
        public void ipcString(String string) throws RemoteException {

        }
    };

    public BinderService() {
    }


    @Nullable
    @Override
    public IBinder onBind(Intent intent) {
        return stub;
    }
}
