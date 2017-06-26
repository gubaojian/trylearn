package com.efurture.file.meta;
import com.efurture.file.io.FormatOutputStream;

import java.io.*;

/**
 * Created by 剑白(jianbai.gbj) on 2017/6/23.
 * 文件索引的信息
 */
public class MetaOutputStream {

    /**
     * 读取文件的内容
     * */
    private FormatOutputStream formatOutputStream = null;

    public MetaOutputStream(String fileName) throws FileNotFoundException {
        this.formatOutputStream = new FormatOutputStream(new BufferedOutputStream(new FileOutputStream(new File(fileName), true)));
    }

    /**
     * 写Meta信息
     * */
    public void  writeMeta(Meta meta) throws IOException {
         meta.write(formatOutputStream);
    }

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
