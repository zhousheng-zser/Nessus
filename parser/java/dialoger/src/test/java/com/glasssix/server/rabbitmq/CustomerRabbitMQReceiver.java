package com.glasssix.server.rabbitmq;

import lombok.extern.slf4j.Slf4j;
import org.springframework.amqp.core.Message;
import org.springframework.amqp.rabbit.annotation.*;
import org.springframework.stereotype.Component;

import java.nio.charset.StandardCharsets;

@Slf4j
@Component
public class CustomerRabbitMQReceiver {

    @RabbitListener(bindings = {@QueueBinding(value = @Queue(value = "${rabbit.server.queue.name}", durable = "true"),
            exchange = @Exchange(value = "${rabbit.server.topicExchange.name}", type = "topic"),
            key = "${rabbit.server.queue.routingKey}")})
    @RabbitHandler
    public void backProcess(Message message){
        log.info("back: "+new String(message.getBody(), StandardCharsets.UTF_8));
    }
}
