package com.efurture.file;

import com.efurture.file.io.Bits;

import java.util.Arrays;

/**
 * Created by 剑白(jianbai.gbj) on 2017/6/28.
 */
public class BitsTest {

    public static void main(String[] args){
        byte[] bts = new byte[8];

        int max = Integer.MAX_VALUE >> 1;
        for(int i=0; i<max; i++){
            int value = i;
            int len = Bits.putInt(bts, value);
            int get = Bits.getInt(bts, 0);
            if(value != get){
                System.out.println(value + "not match " + get);
            }
        }
        System.out.println("int test end");
        long longMax = Long.MAX_VALUE >> 2;
        long step = 1243;
        for(long i=0; i<longMax; i+=step){
            long value = i;
            int len = Bits.putLong(bts, value);
            long get = Bits.getLong(bts, 0);
            if(value != get){
                System.out.println(value + "not match " + get);
            }
        }
        System.out.println("long test end");


    }
}
