package com.glasssix.server.rabbitmq;

import lombok.extern.slf4j.Slf4j;
import org.springframework.amqp.rabbit.connection.CorrelationData;
import org.springframework.amqp.rabbit.core.RabbitTemplate;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

@Slf4j
@Component
public class RabbitMQSender {

    @Autowired
    private RabbitTemplate rabbitTemplate;

    @Autowired
    private RabbitConfig rabbitConfig;


    public void serverSend(String serverRoutingKey,String message,String correlationId){
        CorrelationData correlationData = new CorrelationData(correlationId);
        this.rabbitTemplate.convertAndSend(rabbitConfig.getServerTopicExchange(),serverRoutingKey,message,correlationData);
    }
}
