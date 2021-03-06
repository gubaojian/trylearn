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
     *  数据的状态, 0 代表正常
     * */
    public static final  byte FLAG_NORMAL = 0;

    /**
     * 数据的状态, -1 代表删除
     * */
    public static final  byte FLAG_DELETE = -1;

    /**
     * 默认数据的头部, 不进行压缩
     * */
    public static final  byte DEFAULT_HEADER = 0;

    /**
     * ZIP压缩头部表示
     * */
    public static final  byte GZIP_HEADER = 1;

    /**
     * 数据的头信息, 数据协议的版本号
     * */
    public byte header = DEFAULT_HEADER;

    /**
     * 原始文件名字
     * */
    public String fileName;

    /**
     * 数据的存储的节点
     * */
    public int node = 0;

    /**
     * 标记, 0 代表正常, 1 代表删除
     * */
    public byte flag;

    /**
     * 文件的块信息
     * */
    public List<Block> blocks;


    public static Meta createMeta(byte header, String fileName, int node,  List<Block> blocks){
        Meta meta = new Meta();
        meta.header = header;
        meta.fileName = fileName;
        meta.node = node;
        meta.flag = FLAG_NORMAL;
        meta.blocks = blocks;
        return  meta;
    }


    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;

        Meta meta = (Meta) o;

        if (header != meta.header) return false;
        if (node != meta.node) return false;
        if (flag != meta.flag) return false;
        return blocks.equals(meta.blocks);

    }

    @Override
    public int hashCode() {
        int result = (int) header;
        result = 31 * result + fileName.hashCode();
        result = 31 * result + node;
        result = 31 * result + (int) flag;
        result = 31 * result + blocks.hashCode();
        return result;
    }

    /**
     * 写入Meta信息
     * */
    public void write(FormatOutputStream formatOutputStream ) throws IOException {
        formatOutputStream.writeUByte(header);
        formatOutputStream.writeString(fileName);
        formatOutputStream.writeZInt(node);
        formatOutputStream.writeByte(flag);
        formatOutputStream.writeZInt(blocks.size());
        for(Block block : blocks){
            block.write(formatOutputStream);
        }
    }

    /**
     *  读取Meta信息
     * */
    public boolean read(FormatInputStream formatInputStream) throws IOException {
        byte  header = formatInputStream.readUByte();
        if(header < 0){
            return  false;
        }
        this.header = header;
        this.fileName = formatInputStream.readString();
        this.node  = formatInputStream.readZInt();
        this.flag = formatInputStream.readByte();
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
