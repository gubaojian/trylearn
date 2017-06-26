package com.efurture.file;

/**
 * Created by 剑白(jianbai.gbj) on 2017/6/26.
 */
public class TODO {

    /**
     * 1、读buffer大小优化
     *
     * 2、过期文件清理策略,
     *
     *     1、最简单的清理策略, 直接重新命名文件,然后把老文件的内容copy过去
     *     2、文件的名字生成器,自动生成文件的名字。
     *
     * 后续思考好,在做
     * 2、merge,写延迟文件的性能, 以及自动分片的策略。
     * 3、自动flush线程, 合并流大小优化
     * */
}
