package com.efurture.file;

import java.io.IOException;

/**
 * Created by 剑白(jianbai.gbj) on 2017/6/28.
 */
public class Test4 {

    public static void main(String [] args) throws IOException {
        final  FileStore readDb = new FileStore("store");
        System.out.println(readDb.getString("239"));
        readDb.close();
    }
}
