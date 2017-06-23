package com.efurture.file;

import com.efurture.file.compress.GZip;
import com.efurture.file.io.BlockFileInputStream;
import com.efurture.file.io.BlockOutputStream;
import com.efurture.file.meta.Meta;
import com.efurture.file.meta.MetaOutputStream;
import com.efurture.file.meta.MetaUtils;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.util.Map;

/**
 * С�ļ��洢�����ݿ�,���Դ洢����С�ļ�, �����ļ�������1G��
 * ���С�ļ��ϲ���һ���ļ���, ����Append�����ģʽ, д�벻Ӱ���, �������ܺá�
 * Created by ����(jianbai.gbj) on 2017/6/23.
 */
public class FileDBStore {

    /**
     * �����С
     * */
    public  static final  int BLOCK_BUFFER_SIZE = 1024*1024;

    /**
     * �ļ���д����
     * */
    private BlockOutputStream outputStream;

    /**
     * �ļ���ԭ��Ϣд��
     * */
    private MetaOutputStream metaOutputStream;

    /**
     * ��洢���ļ�
     * */
    private String dbFile;

    /**
     * �ļ�������Ϣ,�����ļ���λ����Ϣ
     * */
    private Map<String, Meta> indexMeta;





    /**
     * ����һ���ļ��Ĵ洢������
     * */
    public FileDBStore(String file) throws IOException{
        String metaFile = file + Meta.FILE_SUFFIX;
        indexMeta = MetaUtils.readMeta(metaFile);
        metaOutputStream = new MetaOutputStream(metaFile);
        outputStream = new BlockOutputStream(file, indexMeta);
        dbFile = file;
    }


    /**
     * �洢�ļ������ݿ���
     * */
    public void put(String fileName, byte[] bts) throws IOException {
        boolean zip = false;
        if(fileName.endsWith(".dat")
                || fileName.endsWith(".txt")
                || fileName.endsWith(".json")
                || fileName.endsWith(".html")
                || fileName.endsWith(".htm")){
            zip = true;
        }
        put(fileName, bts, zip);
    }

    /**
     * �洢�ļ������ݿ���, �Ƿ�ѹ��
     * */
    public void put(String fileName, byte[] bts, boolean zip) throws IOException {
        synchronized (this) {
            indexMeta.remove(fileName);
            int offset = 0;
            if(zip){
                bts =  GZip.compress(bts, 0, bts.length);
            }
            while (offset < bts.length){
                int length = Math.min(bts.length - offset, BLOCK_BUFFER_SIZE);
                outputStream.write(fileName, bts, offset, length);
                offset +=length;
            }
            outputStream.flush();
            if(zip){
                Meta meta = indexMeta.get(fileName);
                meta.version = Meta.GZIP_VERSION;
            }
            metaOutputStream.writeMeta(indexMeta.get(fileName));
            metaOutputStream.flush();
        }
    }

    /**
     * �洢�ַ�������,Ĭ��ѹ���洢
     * */
    public void put(String fileName, String content) throws IOException {
         put(fileName, content.getBytes(), true);
    }

    public void put(String fileName, String content, boolean zip) throws IOException {
        put(fileName, content.getBytes(), zip);
    }

    /**
     * ��ȡ�ļ�,���ļ�������,�򷵻ؿ�, ���ļ�����,�����ļ�������
     * */
    public byte[] get(String fileName) throws IOException {
        Meta meta = indexMeta.get(fileName);
        if(meta == null){
            return  null;
        }
        BlockFileInputStream inputStream = new BlockFileInputStream(dbFile, meta.blocks);
        ByteArrayOutputStream data = new ByteArrayOutputStream(1024*4);
        byte[] buffer = new byte[1024*2];
        int read = 0;
        while ((read = inputStream.read(buffer, 0)) > 0){
            data.write(buffer, 0, read);
        }
        inputStream.close();
        buffer = null;
        if(meta.version == Meta.DEFAULT_VERSION){
            return data.toByteArray();
        }

        if(meta.version == Meta.GZIP_VERSION){
            byte[] bts = data.toByteArray();
            return  GZip.uncompress(bts, 0, bts.length);
        }
        throw  new RuntimeException("Meta Version Not Supported " + meta.version);
    }

    /**
     * ��ȡ�ļ�������,��ת�����ַ���
     * */
    public String getString(String fileName) throws IOException {
        byte[] bts = get(fileName);
        if(bts == null){
            return  null;
        }
        return  new String(bts);

    }

    /**
     * �ر����ݿ�Ĳ���
     * */
    public void close() throws IOException {
        synchronized (this) {
            if(outputStream != null) {
                outputStream.close();
                outputStream = null;
            }
            if(metaOutputStream != null) {
                metaOutputStream.close();
                metaOutputStream = null;
            }
        }
    }

    @Override
    protected void finalize() throws Throwable {
        close();
        super.finalize();
    }
}
