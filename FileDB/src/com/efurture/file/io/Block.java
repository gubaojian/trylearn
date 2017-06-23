package com.efurture.file.io;

import java.io.*;

/**
 * 单个区块, 每个区块代表一个文件,采用边长的block表示
 * Created by 剑白(jianbai.gbj) on 2017/6/23.
 */
public class Block{

    private long off;
    private int len;

    public Block(long off, int len) {
        this.off = off;
        this.len = len;
    }

    public Block() {
    }

    public long getOff() {
        return off;
    }

    public int getLen() {
        return len;
    }




    public void write(FormatOutputStream out) throws IOException {
        out.writeZLong(off);
        out.writeZInt(len);
    }

    public void read(FormatInputStream in) throws IOException{
        off = in.readZLong();
        len = in.readZInt();
    }
}