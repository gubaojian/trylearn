package com.efurture.file.meta;
import com.efurture.file.io.BlockOutputStream;
import com.efurture.file.io.FormatOutputStream;
import com.efurture.file.meta.Meta;

import java.io.*;
import java.util.zip.GZIPOutputStream;

import static com.efurture.file.io.IO.META_BUFFER_SIZE;

/**
 * Created by 剑白(jianbai.gbj) on 2017/6/23.
 * 文件元数据的信息
 */
public class MetaOutputStream {

    /**
     * 元数据持久化
     * */
    private FormatOutputStream formatOutputStream = null;

    public MetaOutputStream(String fileName) throws IOException {
        this.formatOutputStream = new FormatOutputStream(new GZIPOutputStream( new BufferedOutputStream(new FileOutputStream(new File(fileName), true), META_BUFFER_SIZE)));
    }

    /**
     * 写Meta信息
     * */
    public void  writeMeta(Meta meta) throws IOException {
         meta.write(formatOutputStream);
    }

    /**
     * 刷新元数据
     * */
    public void  flush() throws IOException {
        formatOutputStream.flush();
    }

    /**
     * 关闭写入的流信息
     * */
    public void close() throws IOException {
        if (formatOutputStream != null) {
            formatOutputStream.close();
        }
    }
}
