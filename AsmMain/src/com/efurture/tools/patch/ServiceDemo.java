package com.efurture.tools.patch;

/**
 * Created by ½£°×(jianbai.gbj) on 2017/3/14.
 */
public class ServiceDemo {

    public static IncrementChange change;

    public Object getName(){
        if(change != null){
            return change.dispatch(new Object[]{this});
        }
        return  "ServiceName";
    }

    public Object getName(Object argsOne){
        if(change != null){
            return change.dispatch(new Object[]{this, argsOne});
        }
        return  "ServiceName2";
    }
}
