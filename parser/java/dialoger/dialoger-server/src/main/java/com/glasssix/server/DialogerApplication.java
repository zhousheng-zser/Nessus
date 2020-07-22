package com.glasssix.server;

import com.glasssix.algorithm.AlgorithmFactory;
import com.glasssix.common.util.YamlPropertySourceFactory;
import com.glasssix.init.InitAlgorithmProtocol;
import com.google.gson.Gson;
import org.opencv.core.Core;
import org.opencv.core.Mat;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.boot.context.properties.ConfigurationProperties;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.PropertySource;
import org.springframework.context.annotation.Scope;

@PropertySource(value = {"classpath:rabbit.yml","classpath:protocol.yml","pipeline.yml"},factory = YamlPropertySourceFactory.class)
@ConfigurationProperties
@SpringBootApplication
public class DialogerApplication{
	@Value("${algorithm.protocol.config}")
	private String protocolConfigFileName;

	@Value("${algorithm.libParserName}")
	private String libParserName;

	static {
		try {
			System.loadLibrary(Core.NATIVE_LIBRARY_NAME);
		}catch (UnsatisfiedLinkError ignore) {

		}

	}

	public static void main(String[] args) {
		SpringApplication.run(DialogerApplication.class, args);
	}

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
