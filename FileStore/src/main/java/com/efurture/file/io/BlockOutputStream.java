package com.efurture.file.io;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import static com.efurture.file.io.IO.BLOCK_BUFFER_SIZE;

/**
 * Created by 剑白(jianbai.gbj) on 2017/4/15.
 * 多个分片,存储到一个文件中,减少IO开销, 减少并发的读写。
 */
public class BlockOutputStream {



    /**
     * 文件的索引位置
     * */
    private long offset;
    private String fileName;

    private FileOutputStream outputStream;
    private byte[] buffer;
    private int bufferOffset;
    private FlushCallback flushCallback;

    public BlockOutputStream(String fileName) throws FileNotFoundException {
        this.fileName  = fileName;
        if(new File(fileName).exists()) {
            this.offset = (int) new File(fileName).length();
        }
        this.buffer = new byte[BLOCK_BUFFER_SIZE];
    }


    /**
     * 获取文件的大小
     * */
    public long getSize(){
        return  offset;
    }

    /**
     * 写入文件的内容
     * */
    public synchronized List<Block> write(String fileName, byte[] bts, int off, int len) throws IOException {
        int offset = off;
        int end = Math.min(off + len, bts.length);
        List<Block> blocks = new ArrayList<Block>();
        while (offset < end){
            int length = Math.min(end - offset, BLOCK_BUFFER_SIZE);
            blocks.add(writeBlock(fileName, bts, offset, length));
            offset +=length;
        }
        return blocks;
    }

    /**
     * 分块写入文件写入文件的内容, 小文件内容进行合并, bts最大程为block大小
     * */
    protected synchronized Block writeBlock(String fileName, byte[] bts, int off, int len) throws IOException {
        if(bts.length > BLOCK_BUFFER_SIZE){
            throw new IllegalArgumentException("writeBlock buffer size must not larger than block size");
        }
        Block block = new Block(offset, len);
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
            onFlushCallback(false);
            outputStream = null;
            bufferOffset = 0;
        }
        System.arraycopy(bts, off, buffer, bufferOffset, len);
        bufferOffset += len;
        return block;
    }

    /**
     * 刷新磁盘的数据
     * */
    public synchronized void flush() throws IOException {
        if(bufferOffset > 0){
            if(buffer != null){
                getOutputStream().write(buffer, 0, bufferOffset);
                getOutputStream().flush();
                onFlushCallback(false);
                bufferOffset = 0;
            }
        }
    }


    /**
     * 刷新关闭流信息
     * */
    public synchronized void close() throws IOException {
        if(bufferOffset > 0){
            if(buffer != null){
                getOutputStream().write(buffer, 0, bufferOffset);
                getOutputStream().close();
                onFlushCallback(true);
            }
        }
        if(outputStream != null) {
            getOutputStream().close();
            onFlushCallback(true);
        }
        buffer = null;
    }


    public void setFlushCallback(FlushCallback flushCallback) {
        this.flushCallback = flushCallback;
    }

    private  void  onFlushCallback(boolean close) throws IOException {
        if(flushCallback != null){
            flushCallback.onDisk(close);
        }
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
