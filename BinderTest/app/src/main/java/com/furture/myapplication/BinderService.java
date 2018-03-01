package com.furture.myapplication;

import android.app.IntentService;
import android.app.Service;
import android.content.ComponentName;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Binder;
import android.os.IBinder;
import android.os.RemoteException;
import android.support.annotation.Nullable;
import android.util.Log;

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
            Log.e("weex", "server weex current thread "
                    + Thread.currentThread().getName()
             +  android.os.Process.myPid());

            clientService.ipcBytes(bts);
        }

        @Override
        public void ipcString(String string) throws RemoteException {

        }
    };

    private BinderIpc clientService;
    public BinderService() {
    }

    @Override
    public void onCreate() {
        super.onCreate();
        startService(new Intent(getBaseContext(), ClientService.class));
        bindService(new Intent(getBaseContext(), ClientService.class), new ServiceConnection() {
            @Override
            public void onServiceConnected(ComponentName name, IBinder service) {
                clientService  = BinderIpc.Stub.asInterface(service);
            }

            @Override
            public void onServiceDisconnected(ComponentName name) {

            }
        }, BIND_AUTO_CREATE);
    }

    @Override
    public void onDestroy() {
        stopService(new Intent(getBaseContext(), ClientService.class));
        super.onDestroy();
    }

    @Nullable
    @Override
    public IBinder onBind(Intent intent) {
        return stub;
    }
}
