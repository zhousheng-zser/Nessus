package com.glasssix.www;


import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.context.annotation.ComponentScan;

@SpringBootApplication
@ComponentScan(basePackages = {"com.glasssix.www","com.glasssix.server,com.glasssix.common.util"})
public class DialogerApplication {
    public static void main(String[] args) {
        SpringApplication.run(DialogerApplication.class, args);
    }
}
