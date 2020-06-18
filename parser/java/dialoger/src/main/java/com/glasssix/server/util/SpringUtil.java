package com.glasssix.server.util;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.context.ApplicationContext;
import org.springframework.stereotype.Component;

@Component
public class SpringUtil {

    private static ApplicationContext applicationContext;

    @Autowired
    public void SpringUtil(ApplicationContext applicationContext){
        this.applicationContext = applicationContext;
    }
    public static ApplicationContext getApplicationContext() {
        return applicationContext;
    }
}
