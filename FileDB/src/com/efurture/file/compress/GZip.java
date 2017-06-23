package com.efurture.file.compress;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.util.zip.GZIPInputStream;
import java.util.zip.GZIPOutputStream;

/**
 * gzipÑ¹Ëõ·½Ê½, Ä¬ÈÏÑ¹ËõË÷Òý
 * Created by ½£°×(jianbai.gbj) on 2017/6/23.
 */
public class GZip {

    public static  byte[]  compress(byte[] bts, int off, int len) throws IOException {
        ByteArrayOutputStream out = new ByteArrayOutputStream(bts.length);
        GZIPOutputStream outputStream = new GZIPOutputStream(out);
        outputStream.write(bts, off, len);
        outputStream.flush();
        outputStream.finish();
        return out.toByteArray();
    }

    public static byte[]  uncompress(byte[] bts,int off, int len) throws IOException {
        ByteArrayOutputStream baos = new ByteArrayOutputStream(8192);
        GZIPInputStream gis = new GZIPInputStream(new ByteArrayInputStream(bts, off, len));
        int count;
        byte[] buffer = new byte[8192];
        while ((count = gis.read(buffer)) != -1) {
            baos.write(buffer, 0, count);
        }
        gis.close();
        return  baos.toByteArray();
    }
}

