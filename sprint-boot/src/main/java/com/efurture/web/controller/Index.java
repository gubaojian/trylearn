package com.efurture.web.controller;

import java.util.List;

import javax.annotation.Resource;

import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.ResponseBody;

import com.efurture.mybatis.domain.User;
import com.efurture.mybatis.mapper.UserMapper;


/**
 * 注意区分 @RestController 和 @Controller的注解的区别
 * */
@Controller
public class Index{
	
	@Resource
	UserMapper userMapper;

	
	 @RequestMapping("/addUser")
	public   @ResponseBody  List<User> addUser() {
	    	   User user = new User();
	    	   user.setName("dddd");
	    	   userMapper.insert(user);
		   return userMapper.selectAll();
   }
			

    @RequestMapping("/hello")
	public  String hello(Model model) {
    	   List<User> users = userMapper.selectAll();
    	   model.addAttribute("users", users);
    	   return "/hello";
	}
    
    @RequestMapping("/userList")
	public  String userList(Model model) {
    	   List<User> users = userMapper.selectAll();
    	   model.addAttribute("users", users);
    	   return "/userList";
	}
		
}
