package com.efurture.file;

import junit.framework.TestCase;

import java.io.IOException;

/**
 * Created by 剑白(jianbai.gbj) on 2017/6/27.
 */
public class TestPack extends TestCase {


    public void testPack() throws IOException {
        String dir = "store";
        FileStore db = new FileStore(dir);
        db.pack();
        db.close();
    }
}
