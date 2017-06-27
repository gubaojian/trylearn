package com.efurture.file;

import junit.framework.TestCase;

import java.util.LinkedHashMap;
import java.util.Map;

/**
 * Created by 剑白(jianbai.gbj) on 2017/6/27.
 */
public class LinkedHashMapTest extends TestCase {


    public void testLinkedHashMapTest() {

        //LRU
        Map<String, String> map = new LinkedHashMap<String, String>(30, 0.75f, false);
        for (int i = 0; i < 16; i++) {
            map.put(String.valueOf(i), String.valueOf(i + 1));
        }

        for (Map.Entry<String, String> entry : map.entrySet()) {
            System.out.println("key=" + entry.getKey() + "---------value=" + entry.getValue());
        }

        System.out.println("----------华丽的分割线-------------");

        map.get("3");
        map.get("5");
        map.remove("6");
        map.put("6", "100");

        for (Map.Entry<String, String> entry : map.entrySet()) {
            System.out.println("key=" + entry.getKey() + "---------value=" + entry.getValue());
        }
    }
}
