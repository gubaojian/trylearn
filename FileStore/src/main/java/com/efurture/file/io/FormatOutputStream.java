package com.efurture.file.io;

import com.efurture.file.meta.Bits;

import java.io.DataOutputStream;
import java.io.FilterOutputStream;
import java.io.IOException;
import java.io.OutputStream;

/**
 * Created by 剑白(jianbai.gbj) on 2017/6/23.
 */
public class FormatOutputStream extends DataOutputStream{

    private byte[] bts = new byte[8];

    /**
     * Creates a new data output stream to write data to the specified
     * underlying output stream. The counter <code>written</code> is
     * set to zero.
     *
     * @param out the underlying output stream, to be saved for later
     *            use.
     * @see FilterOutputStream#out
     */
    public FormatOutputStream(OutputStream out) {
        super(out);
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
}
