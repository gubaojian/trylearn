package com.efurture.file.meta;

import com.efurture.file.io.Block;
import com.efurture.file.io.FormatInputStream;
import com.efurture.file.io.FormatOutputStream;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

/**
 * Created by ����(jianbai.gbj) on 2017/6/23.
 */
public class Meta {

    public static final String FILE_SUFFIX = ".meta";
    /**
     * Ĭ����ѹ���İ汾
     * */
    public static final  int DEFAULT_VERSION = 0;

    /**
     * Ĭ����ѹ���İ汾
     * */
    public static final  int GZIP_VERSION = 1;

    public int version = DEFAULT_VERSION;
    public String fileName;
    public List<Block> blocks;


    public static Meta createMeta(int version, String fileName, List<Block> blocks){
        Meta meta = new Meta();
        meta.version = version;
        meta.fileName = fileName;
        meta.blocks = blocks;
        return  meta;
    }
    /**
     * �־û�meta����
     * */
    public void write(FormatOutputStream formatOutputStream ) throws IOException {
        formatOutputStream.writeZInt(version);
        formatOutputStream.writeUTF(fileName);
        formatOutputStream.writeZInt(blocks.size());
        for(Block block : blocks){
            block.write(formatOutputStream);
        }
    }

    /**
     * ��ȡmeta����
     * */
    public boolean read(FormatInputStream formatInputStream) throws IOException {
        int  version = formatInputStream.readZInt();
        if(version < 0){
            return  false;
        }
        this.version = version;
        this.fileName = formatInputStream.readUTF();
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
                "version=" + version +
                ", fileName='" + fileName + '\'' +
                ", blocks=" + blocks +
                '}';
    }
}
