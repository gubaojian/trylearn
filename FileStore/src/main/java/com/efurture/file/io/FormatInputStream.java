package com.efurture.file.io;

import java.io.DataInputStream;
import java.io.IOException;
import java.io.InputStream;

/**
 * 自定义格式化输出流
 * Created by (jianbai.gbj) on 2017/6/23.
 */
public class FormatInputStream extends DataInputStream{


    private byte[] bts = new byte[8];

    /**
     * Creates a DataInputStream that uses the specified
     * underlying InputStream.
     *
     * @param in the specified input stream
     */
    public FormatInputStream(InputStream in) {
        super(in);
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
        return (byte)(ch1);
    }
}
