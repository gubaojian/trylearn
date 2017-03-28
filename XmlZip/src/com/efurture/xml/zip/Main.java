package com.efurture.xml.zip;

import org.xml.sax.InputSource;
import org.xml.sax.SAXException;
import org.xml.sax.XMLReader;

import javax.xml.parsers.ParserConfigurationException;
import javax.xml.parsers.SAXParser;
import javax.xml.parsers.SAXParserFactory;
import javax.xml.transform.TransformerConfigurationException;
import java.io.IOException;
import java.io.InputStreamReader;

public class Main {

    public static void main(String[] args) throws ParserConfigurationException, SAXException, TransformerConfigurationException, IOException {
	// write your code here

        InputStreamReader reader = new InputStreamReader(Main.class.getResourceAsStream("/view.xml"));

        SAXParserFactory spf = SAXParserFactory.newInstance();
        SAXParser saxParser = spf.newSAXParser();
        XMLReader xmlReader = saxParser.getXMLReader();
        ZipDefaultHandler zipDefaultHandler = new ZipDefaultHandler();
        zipDefaultHandler.setZipEnable(false);
        xmlReader.setContentHandler(zipDefaultHandler);

        long start = System.currentTimeMillis();
        xmlReader.parse(new InputSource(reader));
        System.out.println((System.currentTimeMillis() - start));


        System.out.println(zipDefaultHandler.getXmlWriter().toString());

    }
}
