package com.efurture.file.io;

import java.io.EOFException;
import java.io.IOException;
import java.io.InputStream;

/**
 * 自定义格式化输出流
 * Created by (jianbai.gbj) on 2017/6/23.
 */
public class FormatInputStream{


    private InputStream in;
    private byte[] bts = new byte[8];

    private long position;

    public FormatInputStream(InputStream in) {
        this.in = in;
        this.position = 0;
    }

    /**
     * 读取指定的字节个数
     * */
    protected  void readFully(byte b[], int off, int len) throws IOException {
        if (len < 0)
            throw new IndexOutOfBoundsException();
        int n = 0;
        while (n < len) {
            int count = in.read(b, off + n, len - n);
            if (count < 0)
                throw new EOFException();
            n += count;
        }
    }

    /**
     * 存储可变长度的int
     * */
    public int readZInt() throws IOException {
        int ch1 = in.read();
        if(ch1 < 0){
            return  ch1;
        }
        bts[0] = (byte) ch1;
        int remain = (ch1 >>> 6 & 0x3); //byte 00000011
        if(remain > 0){
            readFully(bts, 1, remain);
        }
        position += remain + 1;
        return  Bits.getInt(bts, 0);
    }

    /**
     * 读取可变类型的long
     * */
    public long readZLong() throws IOException {
        int ch1 = in.read();
        if(ch1 < 0){
            return  ch1;
        }
        bts[0] = (byte) ch1;
        int remain = (ch1 >>> 5 & 0x7); //byte  00000111
        if(remain > 0){
            readFully(bts, 1, remain);
        }
        position += remain + 1;
        return  Bits.getLong(bts, 0);
    }

    /**
     * 返回负数代表已经读取到文件的末尾
     * */
    public byte readUByte() throws IOException {
        int ch1 = in.read();
        if(ch1 < 0){
            return  -1;
        }
        position += 1;
        return (byte)(ch1);
    }

    /**
     * 读取一个byte
     * */
    public byte readByte() throws IOException {
        int ch = in.read();
        if (ch < 0)
            throw new EOFException();
        position += 1;
        return (byte)(ch);
    }

    public String readString() throws IOException {
        int len = readZInt();
        byte [] bts = new byte[len];
        readFully(bts, 0, bts.length);
        position += len;
        return  new String(bts, IO.STRING_CHARSET);
    }

    public long getPosition() {
        return position;
    }

    /**
     * 关闭流
     * */
    public void  close() throws IOException {
        in.close();
    }



}
