package com.efurture.file.io;

import com.efurture.file.meta.Bits;

import java.io.DataInputStream;
import java.io.IOException;
import java.io.InputStream;

/**
 * Created by ����(jianbai.gbj) on 2017/6/23.
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
     * ���ñ�洢int
     * */
    public int readZInt() throws IOException {
        int ch1 = in.read();
        if(ch1 < 0){
            return  ch1;
        }
        bts[0] = (byte) ch1;
        int remain = (ch1 >>> 6 & 0x3); //byte�ĳ���  00000011
        if(remain > 0){
            readFully(bts, 1, remain);
        }
        return  Bits.getInt(bts, 0);
    }

    /**
     * ���ñ�洢long
     * */
    public long readZLong() throws IOException {
        int ch1 = in.read();
        if(ch1 < 0){
            return  ch1;
        }
        bts[0] = (byte) ch1;
        int remain = (ch1 >>> 5 & 0x7); //byte�ĳ���  00000111
        if(remain > 0){
            readFully(bts, 1, remain);
        }
        return  Bits.getLong(bts, 0);
    }
}
