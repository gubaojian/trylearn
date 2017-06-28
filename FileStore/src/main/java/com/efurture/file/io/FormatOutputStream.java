package com.efurture.file.io;

import java.io.IOException;
import java.io.OutputStream;

/**
 * Created by 剑白(jianbai.gbj) on 2017/6/23.
 */
public class FormatOutputStream {


    private byte[] bts = new byte[8];
    private OutputStream out;

    public FormatOutputStream(OutputStream out) {
        this.out = out;
    }


    /**
     * 存储可变长度的int
     * */
    public  void writeZInt(int val) throws IOException {
        int length = Bits.putInt(bts, val);
        out.write(bts, 0, length);
    }

    /**
     * 存储可变长度的long
     * */
    public  void writeZLong(long val) throws IOException {
        int length = Bits.putLong(bts, val);
        out.write(bts, 0, length);
    }

    /**
     * 写入byte
     * */
    public void writeUByte(byte bt) throws IOException {
        writeByte(bt);
    }

    /**
     * 写入byte
     * */
    public void  writeByte(byte bt) throws IOException {
        out.write(bt);
    }

    /**
     * 写入String, 采用平台独立的编码
     * */
    public void writeString(String str) throws IOException {
        byte[] bts = str.getBytes(IO.STRING_CHARSET);
        writeZInt(bts.length);
        out.write(bts);
    }


    /**
     * 刷新到磁盘
     * */
    public void flush() throws IOException {
        out.flush();
    }

    /**
     * 关闭流
     * */
    public void  close() throws IOException {
        out.close();
    }

}
