package com.efurture.file.io;

import java.io.IOException;

/**
 * 单个区块, 每个区块代表一个文件,采用边长的block表示
 * Created by 剑白(jianbai.gbj) on 2017/6/23.
 */
public class Block{

    private long off;
    private int len;

    public Block(long off, int len) {
        this.off = off;
        this.len = len;
    }

    public Block() {
    }

    public long getOff() {
        return off;
    }

    public int getLen() {
        return len;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;

        Block block = (Block) o;
        if (off != block.off) return false;
        return len == block.len;
    }

    @Override
    public int hashCode() {
        int result = (int) (off ^ (off >>> 32));
        result = 31 * result + len;
        return result;
    }

    public void write(FormatOutputStream out) throws IOException {
        out.writeZLong(off);
        out.writeZInt(len);
    }

    public void read(FormatInputStream in) throws IOException{
        off = in.readZLong();
        len = in.readZInt();
    }

    @Override
    public String toString() {
        return "Block{" +
                "off=" + off +
                ", len=" + len +
                '}';
    }
}