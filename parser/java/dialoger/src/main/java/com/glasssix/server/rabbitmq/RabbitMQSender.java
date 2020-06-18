package com.glasssix.server.rabbitmq;

import lombok.extern.slf4j.Slf4j;
import org.springframework.amqp.rabbit.connection.CorrelationData;
import org.springframework.amqp.rabbit.core.RabbitTemplate;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.UUID;

@Slf4j
@Component
public class RabbitMQSender {

    @Autowired
    private RabbitTemplate rabbitTemplate;

    @Autowired
    private RabbitConfig rabbitConfig;

    public void send(String message){
        CorrelationData correlationData = new CorrelationData(UUID.randomUUID().toString());
        log.info("发送消息:correlationData({}),message({})",correlationData.getId(),message);
        this.rabbitTemplate.convertAndSend(rabbitConfig.getCustomerTopicExchange(),rabbitConfig.getCustomerRoutingKey(),message,correlationData);
    }

    public void back(String serverRoutingKey,String message,String correlationId){
        CorrelationData correlationData = new CorrelationData(correlationId);
        this.rabbitTemplate.convertAndSend(rabbitConfig.getServerTopicExchange(),serverRoutingKey,message,correlationData);
    }
}
