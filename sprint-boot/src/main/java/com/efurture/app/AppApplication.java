package com.efurture.app;

import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;


@SpringBootApplication(scanBasePackages={"com.efurture.mybatis.conf",
		"com.efurture.mustache.conf",
		"com.efurture.web.controller"})
public class AppApplication {


	public static void main(String[] args) {
		SpringApplication.run(AppApplication.class, args);
	}
	
	
	
}
