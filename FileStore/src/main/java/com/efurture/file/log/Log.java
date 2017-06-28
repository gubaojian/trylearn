package com.efurture.file.log;

/**
 * Created by 剑白(jianbai.gbj) on 2017/6/28.
 */
public class Log {

    public static boolean LOG_ENABLE = true;

    public static final void log(String message){
        if(LOG_ENABLE) {
            System.out.println(message);
        }
    }
}
