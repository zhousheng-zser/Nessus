package com.glasssix.server;

import com.glasssix.algorithm.AlgorithmFactory;
import com.glasssix.common.util.YamlPropertySourceFactory;
import com.glasssix.init.InitAlgorithmProtocol;
import com.google.gson.Gson;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.PropertySource;
import org.springframework.context.annotation.Scope;
import org.springframework.stereotype.Component;

//import org.springframework.boot.SpringApplication;
//import org.springframework.boot.autoconfigure.SpringBootApplication;
//import org.springframework.boot.context.properties.ConfigurationProperties;

@PropertySource(value = {"classpath:rabbit.yml","classpath:protocol.yml","classpath:pipeline.yml"},factory = YamlPropertySourceFactory.class)
//@ConfigurationProperties
//@SpringBootApplication
@Component
public class DialogerServerConfig {
	@Value("${algorithm.protocol.config}")
	private String protocolConfigFileName;

	@Value("${algorithm.libParserName}")
	private String libParserName;



/*	public static void main(String[] args) {
		SpringApplication.run(DialogerServerApplication.class, args);
	}*/

	@Bean
	@Scope("prototype")
	public Gson gson(){
		return new Gson();
	}

	@Bean
	public AlgorithmFactory algorithmFactory(){
		return InitAlgorithmProtocol.initProtocol(protocolConfigFileName,libParserName);
	}



}
