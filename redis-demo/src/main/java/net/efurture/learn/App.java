package net.efurture.learn;

import redis.clients.jedis.Jedis;

/**
 * Hello world!
 *
 */
public class App {
    public static void main( String[] args ){
        Jedis jedis = new Jedis("localhost");
        jedis.connect();
        //jedis.auth("redis_furture_gu");
        String key = "world_furture";
        System.out.println(jedis.get(key));
        long start = System.currentTimeMillis();
        for(int i=0; i<100000; i++){
            jedis.set(key + i,  "world will be better" + i + "world will be better,"+ System.currentTimeMillis() +" keep in faith " + System.currentTimeMillis() + " i " + i);
        }
        System.out.println("set used " + (System.currentTimeMillis() - start) + "ms");
        jedis.disconnect();
    }
}
