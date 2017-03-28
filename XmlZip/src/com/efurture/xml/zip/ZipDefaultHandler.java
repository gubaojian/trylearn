package com.efurture.xml.zip;


import org.xml.sax.*;
import org.xml.sax.helpers.AttributesImpl;
import org.xml.sax.helpers.DefaultHandler;

import javax.xml.transform.OutputKeys;
import javax.xml.transform.Result;
import javax.xml.transform.TransformerConfigurationException;
import javax.xml.transform.sax.SAXTransformerFactory;
import javax.xml.transform.sax.TransformerHandler;
import javax.xml.transform.stream.StreamResult;
import java.io.IOException;
import java.io.StringWriter;
import java.util.Properties;

/**
 * Created by 剑白(jianbai.gbj) on 2017/3/28.
 */
public class ZipDefaultHandler extends DefaultHandler {


    /**
     * 元素压缩字典
     * */
    private static Properties elementZipProperties;

    /**
     * 属性压缩字典
     * */
    private static Properties attributesZipProperties;

    private StringWriter xmlWriter;

    private TransformerHandler transformerHandler;

    private boolean zipEnable;


    public ZipDefaultHandler() throws TransformerConfigurationException, IOException {
        if(elementZipProperties == null){
            elementZipProperties = new Properties();
            elementZipProperties.load(ZipDefaultHandler.class.getResourceAsStream("/elements.properties"));
        }

        if(attributesZipProperties == null){
            attributesZipProperties = new Properties();
            attributesZipProperties.load(ZipDefaultHandler.class.getResourceAsStream("/attributes.properties"));
        }
        zipEnable = true;
        resetWriter();
    }

    public void  resetWriter() throws TransformerConfigurationException {
        xmlWriter = new StringWriter();
        Result result = new StreamResult(xmlWriter); //
        SAXTransformerFactory saxTransformerFactory = (SAXTransformerFactory)SAXTransformerFactory.newInstance();
        transformerHandler = saxTransformerFactory.newTransformerHandler();
        transformerHandler.getTransformer().setOutputProperty(OutputKeys.INDENT, "no");
        transformerHandler.setResult(result);

    }

    public void setZipEnable(boolean zipEnable){
        this.zipEnable = zipEnable;
    }

    @Override
    public void startDocument() throws SAXException {
        super.startDocument();
        transformerHandler.startDocument();
    }

    @Override
    public void endDocument() throws SAXException {
        super.endDocument();
    }

    @Override
    public void startElement(String uri, String localName, String qName, Attributes attributes) throws SAXException {
        if(zipEnable){
            if(elementZipProperties.getProperty(qName) != null){
                qName = elementZipProperties.getProperty(qName);
            }
            AttributesImpl transformAttributes = new AttributesImpl();
            for(int i=0; i<attributes.getLength(); i++){
                String attrQName = attributes.getQName(i);
                if(attributesZipProperties.getProperty(attrQName) != null){
                    attrQName = attributesZipProperties.getProperty(attrQName);
                }
                transformAttributes.addAttribute(attributes.getURI(i), attributes.getLocalName(i), attrQName,
                        attributes.getType(i), attributes.getValue(i));
            }
            transformerHandler.startElement(uri, localName, qName, transformAttributes);
        }else{
            transformerHandler.startElement(uri, localName, qName, attributes);
        }
    }

    @Override
    public void endElement(String uri, String localName, String qName) throws SAXException {
        if(zipEnable){
            if(elementZipProperties.getProperty(qName) != null){
                qName = elementZipProperties.getProperty(qName);
            }
        }
        transformerHandler.endElement(uri, localName, qName);
    }

    @Override
    public InputSource resolveEntity(String publicId, String systemId) throws IOException, SAXException {
        return super.resolveEntity(publicId, systemId);
    }

    @Override
    public void startPrefixMapping(String prefix, String uri) throws SAXException {
        transformerHandler.startPrefixMapping(prefix, uri);
    }

    @Override
    public void endPrefixMapping(String prefix) throws SAXException {
        transformerHandler.endPrefixMapping(prefix);
    }

    @Override
    public void notationDecl(String name, String publicId, String systemId) throws SAXException {
        transformerHandler.notationDecl(name, publicId, systemId);
    }

    @Override
    public void unparsedEntityDecl(String name, String publicId, String systemId, String notationName) throws SAXException {
        transformerHandler.unparsedEntityDecl(name, publicId, systemId, notationName);
    }

    @Override
    public void setDocumentLocator(Locator locator) {
        transformerHandler.setDocumentLocator(locator);
    }

    @Override
    public void ignorableWhitespace(char[] ch, int start, int length) throws SAXException {

        transformerHandler.ignorableWhitespace(ch, start, length);
    }

    @Override
    public void skippedEntity(String name) throws SAXException {
        transformerHandler.skippedEntity(name);
    }

    @Override
    public void processingInstruction(String target, String data) throws SAXException {
        transformerHandler.processingInstruction(target, data);
    }

    @Override
    public void characters(char[] chs, int start, int length) throws SAXException {
        int end = start + length -1;
        if(zipEnable){
            char ch = chs[start];
            while (ch == ' ' || ch == '\n' || ch == '\r'){
                start ++;
                ch = chs[start];
            }
            ch = chs[end];
            while (ch == ' ' || ch == '\n' || ch == '\r'){
                if(start > end){
                    break;
                }
                end--;
                ch = chs[end];
            }
        }
        transformerHandler.characters(chs, start, end - start +1);
    }

    @Override
    public void fatalError(SAXParseException e) throws SAXException {
        super.fatalError(e);
    }

    @Override
    public void error(SAXParseException e) throws SAXException {
        super.error(e);
    }

    @Override
    public void warning(SAXParseException e) throws SAXException {
        super.warning(e);
    }

    public StringWriter getXmlWriter() {
        return xmlWriter;
    }
}
