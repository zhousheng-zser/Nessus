package com.glasssix.www;


import lombok.extern.slf4j.Slf4j;
import org.opencv.core.Core;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.boot.builder.SpringApplicationBuilder;
import org.springframework.boot.web.servlet.support.SpringBootServletInitializer;
import org.springframework.context.annotation.ComponentScan;

@Slf4j
@SpringBootApplication
@ComponentScan(basePackages = {"com.glasssix.www","com.glasssix.server,com.glasssix.common.util"})
public class DialogerApplication  extends SpringBootServletInitializer {

    static {
        try {
            System.loadLibrary(Core.NATIVE_LIBRARY_NAME);
        }catch (UnsatisfiedLinkError error){
            System.out.println(">>>>>>>>>>>>>>>>>>>"+error.getMessage());
        }
    }

    @Override
    protected SpringApplicationBuilder configure(SpringApplicationBuilder application) {
        log.info(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
        log.info("user.dir:"+System.getProperty("user.dir"));
        log.info("java.library.path:"+System.getProperty("java.library.path"));
        log.info("classpath:"+System.getenv("classpath"));
        log.info(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
        return application.sources(DialogerApplication.class);
    }

    public static void main(String[] args) {
        SpringApplication.run(DialogerApplication.class, args);
    }
}
