package com.efurture.file.io;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.util.List;

/**
 * Created by 剑白(jianbai.gbj) on 2017/4/15.
 * 读取合并的bucket中某个bucket的数据, 根据blocks块索引信息,进行查找读取。
 * FIXME Buffer 大小优化
 */
public class BlockFileInputStream {

     private RandomAccessFile randomAccessFile;
     private List<Block> blocks;
     private int index;

     /**
      * 读取文件中制定block的内容
      * */
     public BlockFileInputStream(String fileName, List<Block> blocks) throws FileNotFoundException {
        this.randomAccessFile = new RandomAccessFile(fileName, "r");
        this.blocks = blocks;
     }

    /**
     * 每次读取一个block, 返回读取的block的数据, buffer长度必须大于block的长度
     * */
    public int  read(byte[] buffer, int offset) throws IOException {
        if(index >= blocks.size()){
            return  -1;
        }
        Block block = blocks.get(index);
        if((buffer.length - offset) < block.getLen()){
            throw new IllegalArgumentException("buffer size must be big then block size");
        }
        long position = block.getOff();
        randomAccessFile.seek(position);
        int read = 0;
        while (block.getLen() > read){
            int off = read + offset;
            int len = randomAccessFile.read(buffer, off , block.getLen() - read);
            if(len < 0){
                break;
            }
            read +=len;
        }
        index++;
        return  block.getLen();
    }


    /**
     * 关闭流信息
     * */
    public void  close() throws IOException {
        randomAccessFile.close();
        randomAccessFile = null;
        blocks = null;
    }
}
