package com.glasssix.server.rabbitmq;

import lombok.extern.slf4j.Slf4j;
import org.springframework.amqp.rabbit.connection.CorrelationData;
import org.springframework.amqp.rabbit.core.RabbitTemplate;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.UUID;

@Slf4j
@Component
public class CustomerRabbitMQSender {

    @Autowired
    private RabbitTemplate rabbitTemplate;

    @Autowired
    private RabbitConfig rabbitConfig;


    public void customerSend(String customerRoutingKey,String message){
        CorrelationData correlationData = new CorrelationData(UUID.randomUUID().toString());
        log.info("客户发送消息:correlationData({})",correlationData.getId());
        this.rabbitTemplate.convertAndSend(rabbitConfig.getCustomerTopicExchange(),customerRoutingKey,message,correlationData);
    }
}
