package com.efurture.mybatis.conf;

import javax.sql.DataSource;

import org.mybatis.spring.SqlSessionFactoryBean;
import org.mybatis.spring.annotation.MapperScan;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.jdbc.datasource.DataSourceTransactionManager;

@Configuration
@MapperScan(basePackages="com.efurture.mybatis.mapper")
public class MybatisConfig {
	
	@Bean
    public DataSourceTransactionManager transactionManager(DataSource dataSource) {
        return new DataSourceTransactionManager(dataSource);
    }
	
	 @Bean
	public SqlSessionFactoryBean sqlSessionFactory(DataSource dataSource) {
	        SqlSessionFactoryBean factoryBean = new SqlSessionFactoryBean();
	        factoryBean.setDataSource(dataSource);
	        //factoryBean.setConfigLocation(new ClassPathResource("mybatis-config.xml"));
	        return factoryBean;
	}
}
