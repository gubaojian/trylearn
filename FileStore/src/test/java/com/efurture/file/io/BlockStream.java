package com.efurture.file.io;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.RandomAccessFile;

/**
 * Created by 剑白(jianbai.gbj) on 2017/6/28.
 */
public class BlockStream {

    public static void main(String[] args) throws IOException {
        RandomAccessFile randomAccessFile = new RandomAccessFile("store/2.node", "r");
        byte[] bts = new byte[1243];
        randomAccessFile.seek(1108369l);
        randomAccessFile.readFully(bts, 0, bts.length);
        randomAccessFile.close();
        System.out.println(new String(bts));

    }
}
