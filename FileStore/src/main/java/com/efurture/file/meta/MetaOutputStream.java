package com.efurture.file.meta;
import com.efurture.file.io.FormatOutputStream;

import java.io.*;

/**
 * Created by ����(jianbai.gbj) on 2017/6/23.
 * �ļ���������Ϣ
 */
public class MetaOutputStream {

    /**
     * ��ȡ�ļ�������
     * */
    private FormatOutputStream formatOutputStream = null;

    public MetaOutputStream(String fileName) throws FileNotFoundException {
        this.formatOutputStream = new FormatOutputStream(new BufferedOutputStream(new FileOutputStream(new File(fileName), true)));
    }

    /**
     * дMeta��Ϣ
     * */
    public void  writeMeta(Meta meta) throws IOException {
         meta.write(formatOutputStream);
    }

    public void  flush() throws IOException {
        formatOutputStream.flush();
    }

    /**
     * �ر�д�������Ϣ
     * */
    public void close() throws IOException {
        if (formatOutputStream != null) {
            formatOutputStream.close();
        }
    }
}
