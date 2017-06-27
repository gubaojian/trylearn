package com.efurture.file.io;

import java.nio.charset.Charset;

/**
 * Created by 剑白(jianbai.gbj) on 2017/6/27.
 */
class IO {

    /**
     * 存储的编码
     * */
    public static final Charset STRING_CHARSET = Charset.forName("UTF-8");

    /**
     * 缓存大小
     * */
    public  static final  int BLOCK_BUFFER_SIZE = 1024*1024;


    /**
     * 文件元数据缓存区大小
     * */
    public static final int META_BUFFER_SIZE = BLOCK_BUFFER_SIZE/4;


}
