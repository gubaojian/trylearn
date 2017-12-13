package com.efurture.sync;

import java.util.concurrent.*;

public class Main {

    public static void main(String[] args) {
	// write your code here
        System.out.println("weex " + args);
        long start = 0;


        start = System.currentTimeMillis();
        for(int i=0; i<1000; i++){
            async(new Runnable() {
                @Override
                public void run() {
                    try {
                        //Thread.sleep(1);
                    } catch (Exception e) {
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
                        //Thread.sleep(1);
                        latch.countDown();
                    } catch (Exception e) {
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
                latch.await(10, TimeUnit.MILLISECONDS);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
        System.out.println("sync used " + (System.currentTimeMillis() - start));


        start = System.currentTimeMillis();
        for(int i=0; i<10000; i++){
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
                latch.await(10, TimeUnit.MILLISECONDS);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
        System.out.println("CountDownLatch sync used " + (System.currentTimeMillis() - start));
        start = System.currentTimeMillis();
        for(int i=0; i<10000; i++){
            FutureTask task  = new FutureTask(new Runnable() {
                @Override
                public void run() {
                    try {
                        Thread.sleep(1);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
            }, null);
            async(task);
            try {
                task.get(10, TimeUnit.MILLISECONDS);
            } catch (InterruptedException e) {
                e.printStackTrace();
            } catch (ExecutionException e) {
                e.printStackTrace();
            } catch (TimeoutException e) {
                e.printStackTrace();
            }
        }
        System.out.println("FurtureTask sync used " + (System.currentTimeMillis() - start));


        executorService.shutdown();
    }


    public static void  async(Runnable runnable){
        executorService.execute(runnable);
    }


    static  final ExecutorService executorService = Executors.newSingleThreadExecutor();

}
