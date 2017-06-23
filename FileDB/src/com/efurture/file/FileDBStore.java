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
 * 小文件存储的数据库,可以存储海量小文件, 单个文件不超过1G。
 * 多个小文件合并到一个文件中, 基于Append的设计模式, 写入不影响读, 并发性能好。
 * Created by 剑白(jianbai.gbj) on 2017/6/23.
 */
public class FileDBStore {

    /**
     * 缓存大小
     * */
    public  static final  int BLOCK_BUFFER_SIZE = 1024*1024;

    /**
     * 文件块写操作
     * */
    private BlockOutputStream outputStream;

    /**
     * 文件的原信息写入
     * */
    private MetaOutputStream metaOutputStream;

    /**
     * 存存储的文件
     * */
    private String dbFile;

    /**
     * 文件索引信息,包含文件的位置信息
     * */
    private Map<String, Meta> indexMeta;





    /**
     * 创建一个文件的存储管理器
     * */
    public FileDBStore(String file) throws IOException{
        String metaFile = file + Meta.FILE_SUFFIX;
        indexMeta = MetaUtils.readMeta(metaFile);
        metaOutputStream = new MetaOutputStream(metaFile);
        outputStream = new BlockOutputStream(file, indexMeta);
        dbFile = file;
    }


    /**
     * 存储文件到数据库中
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
     * 存储文件到数据库中, 是否压缩
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
     * 存储字符串内容,默认压缩存储
     * */
    public void put(String fileName, String content) throws IOException {
         put(fileName, content.getBytes(), true);
    }

    public void put(String fileName, String content, boolean zip) throws IOException {
        put(fileName, content.getBytes(), zip);
    }

    /**
     * 读取文件,若文件不存在,则返回空, 若文件存在,返回文件的内容
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
     * 获取文件的内容,并转换成字符串
     * */
    public String getString(String fileName) throws IOException {
        byte[] bts = get(fileName);
        if(bts == null){
            return  null;
        }
        return  new String(bts);

    }

    /**
     * 关闭数据库的操作
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
