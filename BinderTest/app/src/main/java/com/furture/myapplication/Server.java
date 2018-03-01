package com.furture.myapplication;

import android.util.Log;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.Semaphore;

/**
 * Created by furture on 2018/3/1.
 */

public class Server extends  Thread {




    private List<Semaphore> shareMap;

    public Server(List<Semaphore> semaphores) throws InterruptedException {
        this.shareMap = semaphores;
        semaphores.get(1).acquire();
    }

    @Override
    public void run() {
        super.run();
        try{
            int read = 0;
            int write = 1;
            while (true){
                shareMap.get(read).acquire();
                //Log.e("weex", "server read");
                shareMap.get(read).release();
                read +=2;
                read = read%ThreadIpc.PAGE_SIZE;
                //Log.e("weex", "server write");
                Semaphore writeSeamphore = shareMap.get(write);

                write +=2;
                write = write%ThreadIpc.PAGE_SIZE;
                shareMap.get(write).acquire();
                writeSeamphore.release();
            }
        }catch (Exception e){
            e.printStackTrace();
        }

    }
}
