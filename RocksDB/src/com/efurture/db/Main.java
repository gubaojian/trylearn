package com.efurture.db;

import org.rocksdb.Options;
import org.rocksdb.RocksDB;
import org.rocksdb.RocksDBException;

public class Main {
    static {
        RocksDB.loadLibrary();
    }

    public static void main(String[] args) throws RocksDBException {
	// write your code here
         long start = System.currentTimeMillis();
        Options options = new Options();
        options.setCreateIfMissing(true);
        RocksDB rocksDB = RocksDB.open(options, "store");
        String str = "Repeatedly creating DataOutputStream and DataInputStream instances is not good for performance.\n" +
                "\n" +
                "However, I suspect that a more important performance issue is that you are reading and writing without any Java-side buffering. This means that each read / write call is making one (and possibly many) syscalls to read data. System calls are a couple of orders of magnitude slower than ordinary Java method calls.\n" +
                "\n" +
                "You need to wrap the Socket input stream in a BufferedInputStream and the output stream in a BufferedOutputStream, and then wrap these in the Data streams. (Note that when you do this, it becomes more important that you flush the DataOutputStream at the end of each message, or series of messages.\n" +
                "\n" +
                "Do I write a few info, and then flush after every each write?\n" +
                "Like I said above, you flush at the end of each message, or each sequence of messages. Each flush on your buffered output stream will result in a syscall, assuming there is data in the buffer to be flushed. If you flush on every write you are (probably) doing unnecessary syscalls.\n" +
                "\n" +
                "It is up to you to figure out where your protocol's message boundaries ought to be. It depends entirely on the details of the data you are sending / receiving, and the way it is processed.";

        for(int i=0; i<100000; i++) {
             rocksDB.put((i + "").getBytes(), str.getBytes()) ;
        }
        rocksDB.get("10000".getBytes());
        rocksDB.close();

        System.out.println("used " + (System.currentTimeMillis() - start));
    }
}
