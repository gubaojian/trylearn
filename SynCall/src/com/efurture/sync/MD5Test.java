package com.efurture.sync;

import com.sun.xml.internal.messaging.saaj.util.ByteOutputStream;

import java.io.*;
import java.math.BigInteger;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLConnection;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

/**
 * Created by 剑白(jianbai.gbj) on 2017/10/21.
 */
public class MD5Test {


    public static void main(String[] args) throws IOException {
        //byte[] code = readFile("/Users/furture/Desktop/server/fcdp20174");



        //System.out.println(md5(code) + " length " + code.length);

        //System.out.println(code[code.length - 1]);
        //showInfo("https://jupage.taobao.com/wow/ju/act/fcdp2017?wx_navbar_transparent=true&wh_weex=true&spm=a2141.1.card6-busi.4&scm=2019.1006.100.4&itemId=520473303277,555419961327&page_inside_embed=true&card_expo_pos=4&_page_inside_embed_=true&_page_home_isweex_=true&wh_native=true&love=444");
        String url = "https://market.m.taobao.com/apps/goodshops/index/recommend-sep.html?spm=a2141.1.goodshop.3&wh_weex=true&wx_navbar_transparent=true&algArgs=sellerUnit_678155967,529386763682-527841431886-527840731573,532875296532&categoryId=3001&showActivityAtmosphere=false&_wx_f_=1&wh_native=true&wh_ttid=native";
        showInfo(url);

        //System.out.println(md5("\n" + (new String(code, "UTF-8")).getBytes("UTF-8")));

    }

    private static void showInfo(String url) throws IOException {
        byte[] code = readUrl(url);
        System.out.println(md5(code) + " length " + code.length);


        System.out.println(new String(code, "UTF-8"));
    }


    private static byte[] readUrl(String urlStr) throws IOException {
        // get URL content
        URL url = new URL(urlStr);
        URLConnection conn = url.openConnection();

        // open the stream and put it into BufferedReader
        ByteArrayOutputStream outputStream = new ByteArrayOutputStream(1024);
        InputStream inputStream = conn.getInputStream();
        byte[] buffer = new byte[1024];
        int length = 0;
        while ((length = inputStream.read(buffer)) >=  0){
            outputStream.write(buffer, 0, length);
        }
        inputStream.close();
        return  outputStream.toByteArray();
    }


    private static byte[]  readFile(String file) throws IOException {
        ByteArrayOutputStream outputStream = new ByteArrayOutputStream(1024);
        InputStream inputStream = new FileInputStream(file);
        byte[] buffer = new byte[1024];
        int length = 0;
        while ((length = inputStream.read(buffer)) >=  0){
            outputStream.write(buffer, 0, length);
        }
        return  outputStream.toByteArray();
    }


    public static String md5(String  template){
        try {
            if(template == null){
                return  "";
            }
            return  md5(template.getBytes("UTF-8"));
        } catch (UnsupportedEncodingException e) {
            return  "";
        }
    }

    public static String md5(byte[] bts){
        try {
            MessageDigest digest = MessageDigest.getInstance("MD5");
            digest.update(bts);
            BigInteger bigInt = new BigInteger(1, digest.digest());
            return  bigInt.toString(16);
        } catch (NoSuchAlgorithmException e) {;
            return  "";
        }
    }
}
