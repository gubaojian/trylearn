package com.efurture.file.meta;

import com.efurture.file.io.Block;
import com.efurture.file.io.FormatInputStream;
import com.efurture.file.io.FormatOutputStream;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

/**
 * 文件元信息
 * Created by (jianbai.gbj) on 2017/6/23.
 */
public class Meta {

    /**
     * 默认数据的头部, 不进行压缩
     * */
    public static final  int DEFAULT_HEADER = 0;

    /**
     * ZIP压缩头部表示
     * */
    public static final  int GZIP_HEADER = 1;

    /**
     * 数据的头信息
     * */
    public int header = DEFAULT_HEADER;

    /**
     * 数据的存储的节点
     * */
    public int node = 0;

    /**
     * 原始文件名字
     * */
    public String fileName;

    /**
     * 文件的块信息
     * */
    public List<Block> blocks;


    public static Meta createMeta(int node, int header, String fileName, List<Block> blocks){
        Meta meta = new Meta();
        meta.node = node;
        meta.header = header;
        meta.fileName = fileName;
        meta.blocks = blocks;
        return  meta;
    }
    /**
     * 写入Meta信息
     * */
    public void write(FormatOutputStream formatOutputStream ) throws IOException {
        formatOutputStream.writeZInt(header);
        formatOutputStream.writeUTF(fileName);
        formatOutputStream.writeZInt(node);
        formatOutputStream.writeZInt(blocks.size());
        for(Block block : blocks){
            block.write(formatOutputStream);
        }
    }

    /**
     *  读取Meta信息
     * */
    public boolean read(FormatInputStream formatInputStream) throws IOException {
        int  header = formatInputStream.readZInt();
        if(header < 0){
            return  false;
        }
        this.header = header;
        this.fileName = formatInputStream.readUTF();
        this.node  = formatInputStream.readZInt();
        int length = formatInputStream.readZInt();
        List<Block> blocks = new ArrayList<Block>(length);
        for(int b=0; b<length; b++){
            Block block = new Block();
            block.read(formatInputStream);
            blocks.add(block);
        }
        this.blocks = blocks;
        return  true;
    }

    @Override
    public String toString() {
        return "Meta{" +
                "header=" + header +
                ", fileName='" + fileName + '\'' +
                ", blocks=" + blocks +
                '}';
    }
}
