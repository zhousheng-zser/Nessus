package com.glasssix.server.threadPool;


import lombok.extern.slf4j.Slf4j;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.scheduling.annotation.EnableAsync;
import org.springframework.scheduling.concurrent.ThreadPoolTaskExecutor;

import java.util.concurrent.Executor;
import java.util.concurrent.ThreadPoolExecutor;

@EnableAsync
@Slf4j
@Configuration
public class ThreadPoolConfig {

    @Value("${spring.thread-pool.core-pool-size}")
    private int corePoolSize;

    @Value("${spring.thread-pool.max-pool-size}")
    private int maxPoolSize;

    @Value("${spring.thread-pool.queue-capacity}")
    private int queueCapacity;

    @Value("${spring.thread-pool.keep-alive-seconds}")
    private int keepAliveSeconds;

    @Value("${spring.thread-pool.thread-name-prefix}")
    private String threadNamePrefix;

    @Bean("taskExecutor")
    public Executor taskExecutor(){
        ThreadPoolTaskExecutor executor = new ThreadPoolTaskExecutor();
        executor.setCorePoolSize(corePoolSize);
        executor.setMaxPoolSize(maxPoolSize);
        executor.setQueueCapacity(queueCapacity);
        executor.setKeepAliveSeconds(keepAliveSeconds);
        executor.setThreadNamePrefix(threadNamePrefix);
        executor.setRejectedExecutionHandler(new ThreadPoolExecutor.CallerRunsPolicy());
        executor.setWaitForTasksToCompleteOnShutdown(true);
        executor.setAwaitTerminationSeconds(60);
        executor.initialize();
        return executor;
    }
}
