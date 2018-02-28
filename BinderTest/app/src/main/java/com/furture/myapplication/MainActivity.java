package com.furture.myapplication;

import android.content.ComponentName;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Toast;


/**
 * https://www.zhihu.com/question/39440766
 *
 * binder 源代码。
 * http://www.angryredplanet.com/~hackbod/openbinder/docs/html/main.html#OpenBinder
 *
 * binder c++
 *  https://android.googlesource.com/platform/system/tools/aidl/+/brillo-m10-dev/docs/aidl-cpp.md
 *
 * */
public class MainActivity extends AppCompatActivity {




    BinderService.HelloBinder localBinder;


    BinderIpcCustom binderIpc;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        Intent intent = new Intent(this, BinderService.class);
        startService(intent);
        bindService(intent, new ServiceConnection() {
            @Override
            public void onServiceConnected(ComponentName name, IBinder service) {
                System.out.println(service);
                if(service instanceof BinderService.HelloBinder) {
                    localBinder = (BinderService.HelloBinder) service;
                }else{
                    binderIpc = BinderIpcCustom.Stub.asInterface(service);
                }
            }

            @Override
            public void onServiceDisconnected(ComponentName name) {

            }
        }, BIND_AUTO_CREATE);
        final byte[] bts = new byte[32];
        final String str = new String(bts);
        findViewById(R.id.binder_ipc_one).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                try{
                    if(localBinder != null) {
                        long start = System.currentTimeMillis();
                        for (int i = 0; i < 1000; i++) {
                            localBinder.sayIpc(bts);
                        }
                        Toast.makeText(getBaseContext(), "used " + (System.currentTimeMillis() - start) + "ms", Toast.LENGTH_SHORT).show();
                    }else{
                        long start = System.currentTimeMillis();
                        //for (int i = 0; i < 1000; i++) {
                            binderIpc.ipcBytes(bts);
                       // }
                        Log.e("weex", "ipc used " + (System.currentTimeMillis() - start) + "ms");

                        start = System.currentTimeMillis();
                        for (int i = 0; i < 1000; i++) {
                        binderIpc.ipcString(str);
                         }
                        Log.e("weex", "ipc used " + (System.currentTimeMillis() - start) + "ms");

                    }
                }catch (Exception e){
                    Log.e("weex", "weex ipc ", e);
                }
            }
        });

        findViewById(R.id.binder_ipc_native).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Log.e("weex", "NativeIpc Start");
                NativeIpc.ipc(bts);
                Log.e("weex", "NativeIpc End");
            }
        });
    }

    @Override
    protected void onDestroy() {
        NativeIpc.exit();
        super.onDestroy();
    }
}
