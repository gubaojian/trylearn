package com.efurture.file;

import junit.framework.TestCase;

/**
 * Created by (jianbai.gbj) on 2017/6/26.
 */
public class Test {
    public static void main(String [] args){
        int max = Integer.MAX_VALUE & 0x3FFFFFFF;
        System.out.println("worker " + (max/(1024*1024)));
    }
}
