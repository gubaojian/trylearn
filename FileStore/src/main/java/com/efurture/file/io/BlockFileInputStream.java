package com.efurture.file.io;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.util.List;

/**
 * Created by 剑白(jianbai.gbj) on 2017/4/15.
 * 根据blocks块索引信息,对文件内容进行读取,返回读取的块信息
 */
public class BlockFileInputStream {

     private RandomAccessFile randomAccessFile;
     private List<Block> blocks;
     private int index;
     private int blockRead;

     /**
      * 读取文件中制定block的内容
      * */
     public BlockFileInputStream(String fileName, List<Block> blocks) throws FileNotFoundException {
        this.randomAccessFile = new RandomAccessFile(fileName, "r");
        this.blocks = blocks;
     }

    /**
     * 每次读取一个block, 返回读取的block的数据, 若buffer大于block,则返回读取的字节数量
     * */
    public int  read(byte[] buffer, int offset) throws IOException {
        if(index >= blocks.size()){
            blockRead = 0;
            return  -1;
        }
        Block block = blocks.get(index);
        long position = block.getOff();
        randomAccessFile.seek(position + blockRead);

        int readLen = Math.min(buffer.length-offset, block.getLen() - blockRead);
        int count = randomAccessFile.read(buffer, offset , readLen);
        if(count < 0){
            index ++;
            return  count;
        }
        blockRead += count;
        if(blockRead >= block.getLen()){
            index++;
            blockRead = 0;
        }
        return  count;
    }


    /**
     * 关闭流信息
     * */
    public void  close() throws IOException {
        blockRead = 0;
        index = 0;
        randomAccessFile.close();
        randomAccessFile = null;
        blocks = null;
    }
}
