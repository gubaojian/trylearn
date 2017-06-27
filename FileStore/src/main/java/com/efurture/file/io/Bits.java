package com.efurture.file.io;

/**
 * 可变int类型存储
 * Created by 剑白(jianbai.gbj) on 2017/4/11.
 */
public class Bits {

    public  static int getInt(byte[] buffer, int off) {
        int len = 1  + (buffer[off] >>> 6 & 0x3); //byte的长度  00000011
        if (len == 1) {
            return ((buffer[off] & 0x3F));
        }

        if (len == 2) {
            return ((buffer[off + 1] & 0xFF)      ) +
                    ((buffer[off + 0] & 0x3F) <<  8);
        }

        if (len == 3) {
            return ((buffer[off + 2] & 0xFF)) +
                    ((buffer[off + 1] & 0xFF) <<  8) +
                    ((buffer[off] & 0x3F) << 16);
        }

        return ((buffer[off + 3] & 0xFF)      ) +
                ((buffer[off + 2] & 0xFF) <<  8) +
                ((buffer[off + 1] & 0xFF) << 16) +
                ((buffer[off]  & 0x3F      ) << 24);
    }


    public  static int putInt(byte[] buffer, int val) {
        if (val >>> 6 == 0) {  //32,高2存储长度的差值, 最低支持
            buffer[0] = (byte) (val | 0x00); //00000000  1位
            return 1;
        }

        if (val >>> 14 == 0) {  //40,高2存储长度的差值, 最低支持
            buffer[1] = (byte) (val);
            buffer[0] = (byte) ((val >>> 8) | 0x40); //01000000  2位
            return 2;
        }

        if (val >>> 22 == 0) {  //40,高2存储长度的差值, 最低支持
            buffer[2] = (byte) (val);
            buffer[1] = (byte) (val >>> 8);
            buffer[0] = (byte) ((val >>> 16) | 0x80); //10000000  3位
            return 3;
        }

        buffer[3] = (byte) (val       );
        buffer[2] = (byte) (val >>>  8);
        buffer[1] = (byte) (val >>> 16);
        buffer[0] = (byte) (val >>> 24 | 0xc0);   //11
        return  4;
    }


    public  static long getLong(byte[] buffer, int off) {
        int len = 1  + (buffer[off] >>> 5 & 0x7); //byte的长度  00000011


        if(len == 1){
            return  (((long) (buffer[off] & 0x1F)));
        }


        if(len == 2){
            return  ((buffer[off + 1] & 0xFFL)) +
                    (((long) (buffer[off] & 0x1F))      << 8);
        }

        if(len == 3){
            return  ((buffer[off + 2] & 0xFFL)) +
                    ((buffer[off + 1] & 0xFFL) << 8) +
                    (((long) (buffer[off] & 0x1F))      << 18);
        }


        if(len == 4){
            return  ((buffer[off + 3] & 0xFFL)) +
                    ((buffer[off + 2] & 0xFFL) << 8) +
                    ((buffer[off + 1] & 0xFFL) << 16) +
                    (((long) (buffer[off] & 0x1F))      << 24);
        }


        if(len == 5){
            return  ((buffer[off + 4] & 0xFFL)) +
                    ((buffer[off + 3] & 0xFFL) << 8) +
                    ((buffer[off + 2] & 0xFFL) << 16) +
                    ((buffer[off + 1] & 0xFFL) << 24) +
                    (((long) (buffer[off] & 0x1F))      << 32);
        }


        if(len == 6){
            return ((buffer[off + 5] & 0xFFL)) +
                    ((buffer[off + 4] & 0xFFL) << 8) +
                    ((buffer[off + 3] & 0xFFL) << 16) +
                    ((buffer[off + 2] & 0xFFL) << 24) +
                    ((buffer[off + 1] & 0xFFL) << 32) +
                    (((long) (buffer[off] & 0x1F))      << 40);
        }

        if(len == 7){
            return
                    ((buffer[off + 6] & 0xFFL)) +
                    ((buffer[off + 5] & 0xFFL) << 8) +
                    ((buffer[off + 4] & 0xFFL) << 16) +
                    ((buffer[off + 3] & 0xFFL) << 24) +
                    ((buffer[off + 2] & 0xFFL) << 32) +
                    ((buffer[off + 1] & 0xFFL) << 40) +
                    (((long) (buffer[off] & 0x1F))      << 48);
        }


        return ((buffer[off + 7] & 0xFFL)      ) +
                ((buffer[off + 6] & 0xFFL) <<  8) +
                ((buffer[off + 5] & 0xFFL) << 16) +
                ((buffer[off + 4] & 0xFFL) << 24) +
                ((buffer[off + 3] & 0xFFL) << 32) +
                ((buffer[off + 2] & 0xFFL) << 40) +
                ((buffer[off + 1] & 0xFFL) << 48) +
                (((long) (buffer[off] & 0x1F))      << 56);

    }

    public  static int putLong(byte[] buffer, long val) {
        if (val >>> 5 == 0) {
            buffer[0] = (byte) ((val      | 0x00));
            return 1;
        }

        if (val >>> 13 == 0) {
            buffer[1] = (byte) (val       );
            buffer[0] = (byte) ((val >>> 8 | 0x20));
            return 2;
        }


        if (val >>> 21 == 0) {
            buffer[2] = (byte) (val       );
            buffer[1] = (byte) (val >>>  8);
            buffer[0] = (byte) ((val >>> 16|0x40));
            return 3;
        }

        if (val >>> 29 == 0) {
            buffer[3] = (byte) (val       );
            buffer[2] = (byte) (val >>>  8);
            buffer[1] = (byte) (val >>> 16);
            buffer[0] = (byte) ((val >>> 24| 0x60));
            return 4;
        }


        if (val >>> 37 == 0) {
            buffer[4] = (byte) (val       );
            buffer[3] = (byte) (val >>>  8);
            buffer[2] = (byte) (val >>> 16);
            buffer[1] = (byte) (val >>> 24);
            buffer[0] = (byte) ((val >>> 32 | 0x80));
            return 5;
        }


        if (val >>> 45 == 0) {
            buffer[5] = (byte) (val       );
            buffer[4] = (byte) (val >>>  8);
            buffer[3] = (byte) (val >>> 16);
            buffer[2] = (byte) (val >>> 24);
            buffer[1] = (byte) (val >>> 32);
            buffer[0] = (byte) ((val >>> 40 | 0xa0));
            return 6;
        }


        if (val >>> 53 == 0) {
            buffer[6] = (byte) (val       );
            buffer[5] = (byte) (val >>>  8);
            buffer[4] = (byte) (val >>> 16);
            buffer[3] = (byte) (val >>> 24);
            buffer[2] = (byte) (val >>> 32);
            buffer[1] = (byte) (val >>> 40);
            buffer[0] =  (byte) ((val >>> 48) | 0xc0);
            return 7;
        }

        buffer[7] = (byte) (val       );
        buffer[6] = (byte) (val >>>  8);
        buffer[5] = (byte) (val >>> 16);
        buffer[4] = (byte) (val >>> 24);
        buffer[3] = (byte) (val >>> 32);
        buffer[2] = (byte) (val >>> 40);
        buffer[1] = (byte) (val >>> 48);
        buffer[0] = (byte) ((val >>> 56) | 0xe0);
        return  8;
    }

}
