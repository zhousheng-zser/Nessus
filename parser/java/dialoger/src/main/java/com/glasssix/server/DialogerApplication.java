package com.glasssix.server;

import com.glasssix.server.util.YamlPropertySourceFactory;
import com.google.gson.Gson;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.boot.context.properties.ConfigurationProperties;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.PropertySource;
import org.springframework.context.annotation.Scope;

@PropertySource(value = {"classpath:rabbit.yml","classpath:protocol.yml"},factory = YamlPropertySourceFactory.class)
@ConfigurationProperties
@SpringBootApplication
public class DialogerApplication{

	public static void main(String[] args) {
		SpringApplication.run(DialogerApplication.class, args);
	}

	@Bean
	@Scope("prototype")
	public Gson gson(){
		return new Gson();
	}

}
