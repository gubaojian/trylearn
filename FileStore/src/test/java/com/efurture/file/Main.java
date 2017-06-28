package com.efurture.file;


import java.io.IOException;
import java.util.concurrent.CountDownLatch;

public class Main {


    public static void main(String[] args) throws IOException, InterruptedException {
	// write your code here

        long start = System.currentTimeMillis();
        final FileStore db = new FileStore("store");
        System.out.println("create store used " + (System.currentTimeMillis() - start));
        db.setWriteSyn(false);
        final String str = "Repeatedly creating DataOutputStream and DataInputStream instances is not good for performance.\n" +
                "\n" +
                "However, I suspect that a more important performance issue is that you are reading and writing without any Java-side buffering. This means that each read / write call is making one (and possibly many) syscalls to read data. System calls are a couple of orders of magnitude slower than ordinary Java method calls.\n" +
                "\n" +
                "You need to wrap the Socket input stream in a BufferedInputStream and the output stream in a BufferedOutputStream, and then wrap these in the Data streams. (Note that when you do this, it becomes more important that you flush the DataOutputStream at the end of each message, or series of messages.\n" +
                "\n" +
                "Do I write a few info, and then flush after every each write?\n" +
                "Like I said above, you flush at the end of each message, or each sequence of messages. Each flush on your buffered output stream will result in a syscall, assuming there is data in the buffer to be flushed. If you flush on every write you are (probably) doing unnecessary syscalls.\n" +
                "\n" +
                "It is up to you to figure out where your protocol's message boundaries ought to be. It depends entirely on the details of the data you are sending / receiving, and the way it is processed.";

        final CountDownLatch latch = new CountDownLatch(10);

        for(int c=0; c<10; c++){
            new Thread(new Runnable() {
                @Override
                public void run() {
                    try {
                        for(int i=0; i<10000; i++) {
                                db.put(i + "", str + i, false);
                        }
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                    System.out.println("react end");
                    latch.countDown();
                }
            }).start();
        }

        latch.await();

        db.close();

        System.out.println("used " + (System.currentTimeMillis() - start));
    }
}
