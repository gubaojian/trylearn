package com.efurture.mustache.conf;

import java.lang.reflect.Field;

import javax.annotation.PostConstruct;
import javax.annotation.Resource;

import org.springframework.boot.autoconfigure.mustache.web.MustacheViewResolver;
import org.springframework.context.annotation.Configuration;
import org.springframework.util.ReflectionUtils;

import com.samskivert.mustache.Mustache;
import com.samskivert.mustache.Mustache.Compiler;

@Configuration
//@EnableConfigurationProperties(MustacheProperties.class)
public class MustacheConfig{

	
	@Resource
	MustacheViewResolver mustacheViewResolver;
	


	@PostConstruct
	public void configMustache(){
		try{
			Field compilerField = ReflectionUtils.findField(MustacheViewResolver.class, "compiler");
			Field missingIsNullField  = ReflectionUtils.findField(Mustache.Compiler.class, "missingIsNull");
			Field nullValueField  = ReflectionUtils.findField(Mustache.Compiler.class, "nullValue");

			compilerField.setAccessible(true);
			nullValueField.setAccessible(true);
			missingIsNullField.setAccessible(true);
			Mustache.Compiler compiler = (Compiler) ReflectionUtils.getField(compilerField, mustacheViewResolver);
			ReflectionUtils.setField(missingIsNullField, compiler, true);
			ReflectionUtils.setField(nullValueField, compiler, "");
		}catch(Exception e){
			throw new RuntimeException(e);
		}
	}


		
}
