package com.efurture.file.io;

import java.io.*;

/**
 * ��������, ÿ���������һ���ļ�,���ñ߳���block��ʾ
 * Created by ����(jianbai.gbj) on 2017/6/23.
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