package com.glasssix.www;


import org.opencv.core.Core;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.context.annotation.ComponentScan;

@SpringBootApplication
@ComponentScan(basePackages = {"com.glasssix.www","com.glasssix.server,com.glasssix.common.util"})
public class DialogerApplication {

    static {
        try {
            System.loadLibrary(Core.NATIVE_LIBRARY_NAME);
        }catch (UnsatisfiedLinkError error){
            System.out.println(">>>>>>>>>>>>>>>>>>>"+error.getMessage());
        }
    }

    public static void main(String[] args) {
        SpringApplication.run(DialogerApplication.class, args);
    }
}
