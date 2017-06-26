package com.efurture.file.io;

import com.efurture.file.FileStore;
import com.efurture.file.meta.Meta;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Map;

/**
 * Created by 剑白(jianbai.gbj) on 2017/4/15.
 * 多个分片,存储到一个文件中,减少IO开销, 减少并发的读写。
 */
public class BlockOutputStream {

    /**
     * 缓存大小
     * */
    public  static final  int BLOCK_BUFFER_SIZE = 1024*1024;


    /**
     * 文件的索引位置
     * */
    private long offset;
    private String fileName;

    private FileOutputStream outputStream;
    private byte[] buffer;
    private int bufferOffset;
    private Map<String, Meta> indexMeta;


    public BlockOutputStream(String fileName, Map<String, Meta> indexMeta) throws FileNotFoundException {
        this.fileName  = fileName;
        this.indexMeta = indexMeta;
        this.offset = (int)new File(fileName).length();
    }

    /**
     * 写入文件的内容
     * */
    public synchronized void write(String fileName, byte[] bts, int off, int len) throws IOException {
        int offset = off;
        int end = Math.min(off + len, bts.length);
        while (offset < end){
            int length = Math.min(end - offset, BLOCK_BUFFER_SIZE);
            writeBlock(fileName, bts, offset, length);
            offset +=length;
        }
    }

    /**
     * 分块写入文件写入文件的内容, 小文件内容进行合并, bts最大程为block大小
     * */
    protected synchronized void writeBlock(String fileName, byte[] bts, int off, int len) throws IOException {
        if(buffer == null){
            buffer = new byte[BLOCK_BUFFER_SIZE];
        }
        if(bts.length > BLOCK_BUFFER_SIZE){
            throw new IllegalArgumentException("writeBlock buffer size must not larger than block size");
        }
        Meta meta =  this.indexMeta.get(fileName);
        if(meta == null){
            meta = Meta.createMeta(Meta.DEFAULT_VERSION, fileName, new ArrayList<Block>(2));
            this.indexMeta.put(fileName, meta);
        }
        Block block = new Block(offset, len);
        meta.blocks.add(block);
        offset += len;
        if ((bufferOffset + len) > buffer.length) {
            int remain = buffer.length - bufferOffset;
            if (remain > 0) {
                System.arraycopy(bts, off, buffer, bufferOffset, remain);
                off += remain;
                len -= remain;
                bufferOffset += remain;
            }
            getOutputStream().write(buffer, 0, bufferOffset);
            getOutputStream().close();
            outputStream = null;
            bufferOffset = 0;
        }
        System.arraycopy(bts, off, buffer, bufferOffset, len);
        bufferOffset += len;
    }

    public synchronized void flush() throws IOException {
        if(bufferOffset > 0){
            if(buffer != null){
                getOutputStream().write(buffer, 0, bufferOffset);
                getOutputStream().close();
                outputStream = null;
                bufferOffset = 0;
            }
        }
    }


    public synchronized void close() throws IOException {
        if(bufferOffset > 0){
            if(buffer != null){
                getOutputStream().write(buffer, 0, bufferOffset);
                getOutputStream().close();
                outputStream = null;
            }
        }
        outputStream = null;
        buffer = null;
    }

    private FileOutputStream getOutputStream() throws FileNotFoundException {
        if(outputStream == null){
            File file = new File(fileName);
            if(file.getParentFile() != null){
                 if(!file.getParentFile().exists()){
                     file.getParentFile().mkdirs();
                 }
            }
            outputStream = new FileOutputStream(fileName, true);
        }
        return  outputStream;
    }

}
