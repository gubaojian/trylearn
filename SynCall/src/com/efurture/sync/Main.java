package com.efurture.sync;

import java.util.concurrent.CountDownLatch;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

public class Main {

    public static void main(String[] args) {
	// write your code here
        System.out.println("weex " + args);

        long start = System.currentTimeMillis();
        for(int i=0; i<1000; i++){
            async(new Runnable() {
                @Override
                public void run() {
                    try {
                        Thread.sleep(1);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
            });
            try {
                Thread.sleep(4);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
        System.out.println("async used " + (System.currentTimeMillis() - start));
        start = System.currentTimeMillis();
        for(int i=0; i<1000; i++){
            final CountDownLatch latch = new CountDownLatch(1);
            async(new Runnable() {
                @Override
                public void run() {
                    try {
                        Thread.sleep(1);
                        latch.countDown();
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
            });
            try {
                Thread.sleep(4);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            try {
                latch.await(10, TimeUnit.MICROSECONDS);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
        System.out.println("sync used " + (System.currentTimeMillis() - start));
        executorService.shutdown();
    }


    public static void  async(Runnable runnable){
        executorService.execute(runnable);
    }


    static  final ExecutorService executorService = Executors.newSingleThreadExecutor();

}
