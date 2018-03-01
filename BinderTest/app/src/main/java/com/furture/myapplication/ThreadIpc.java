package com.furture.myapplication;

import android.util.Log;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.Semaphore;

/**
 * Created by furture on 2018/3/1.
 */

public class ThreadIpc {
    public static final int PAGE_SIZE = 16;

    private List<Semaphore> semaphores;

    private int read = 1;
    private int write = 0;

    public ThreadIpc() throws InterruptedException {
        this.semaphores = startIpcServer();
    }

    public void loop(int times) throws InterruptedException {
        for(int i=0; i<times; i++){
            loop();
        }
    }

    public void loop() throws InterruptedException {
        Semaphore semaphore = semaphores.get(write);
        //Log.e("weex", "client write");
        write +=2;
        write = write%ThreadIpc.PAGE_SIZE;
        semaphores.get(write).acquire();
        semaphore.release();

        semaphores.get(read).acquire();
        //Log.e("weex", "client read");
        semaphores.get(read).release();

        read +=2;
        read = read%ThreadIpc.PAGE_SIZE;
    }

    private static List<Semaphore> startIpcServer() throws InterruptedException {
        List<Semaphore> semaphores = createSemphore(PAGE_SIZE);
        semaphores.get(0).acquire();
        Server server = new Server(semaphores);
        server.start();
        return semaphores;
    }


    private static List<Semaphore> createSemphore(int size){
        List<Semaphore> shareMap = new ArrayList<>(size);
        for(int i=0; i<size; i++){
            shareMap.add(new Semaphore(1));
        }
        return shareMap;
    }
}
