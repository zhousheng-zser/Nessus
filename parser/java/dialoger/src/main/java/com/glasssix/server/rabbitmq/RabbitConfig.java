package com.glasssix.server.rabbitmq;

import lombok.extern.slf4j.Slf4j;
import org.springframework.amqp.core.*;
import org.springframework.amqp.rabbit.connection.CachingConnectionFactory;
import org.springframework.amqp.rabbit.connection.CorrelationData;
import org.springframework.amqp.rabbit.core.RabbitTemplate;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;

import java.util.HashMap;
import java.util.Map;

@Slf4j
@Configuration
public class RabbitConfig {

    @Value("${rabbit.customer.topicExchange.name}")
    private String CustomerTopicExchange;

    @Value("${rabbit.customer.queue.name}")
    private  String CustomerQueue;

    @Value("${rabbit.customer.queue.routingKey}")
    private  String CustomerRoutingKey;

    @Value("${rabbit.server.topicExchange.name}")
    private String ServerTopicExchange;

    @Value("${rabbit.server.queue.name}")
    private String ServerQueue;

    @Value("${rabbit.server.queue.routingKey}")
    private String ServerRoutingKey;

    @Autowired
    private CachingConnectionFactory connectionFactory;



    @Bean
    public Queue createCustomerQueue(){
        Map<String,Object> args = new HashMap<String,Object>();
        /*args.put("x-message-ttl",10000);
        args.put("x-max-length",1000);
        args.put("x-max-length-bytes",1024*1024*1024);*/
        return new Queue(CustomerQueue,true,false,false,args);
    }

    @Bean
    public TopicExchange createCustomerExchange(){
        return new TopicExchange(CustomerTopicExchange);
    }

    @Bean
    public Binding bindCustomerQueue(){
        return BindingBuilder.bind(createCustomerQueue())
                .to(createCustomerExchange())
                .with(CustomerRoutingKey);
    }

    @Bean
    public Queue createServerQueue(){
        return new Queue(ServerQueue);
    }

    @Bean
    public TopicExchange createServerExchange(){
        return new TopicExchange(ServerTopicExchange);
    }

    @Bean
    public Binding bindServerQueue(){
        return BindingBuilder.bind(createServerQueue())
                .to(createServerExchange())
                .with(ServerRoutingKey);
    }

    @Bean
    public RabbitTemplate rabbitTemplate(){
        connectionFactory.setPublisherConfirmType(CachingConnectionFactory.ConfirmType.CORRELATED);
        connectionFactory.setPublisherReturns(true);
        RabbitTemplate rabbitTemplate = new RabbitTemplate(connectionFactory);
        rabbitTemplate.setMandatory(true);

        rabbitTemplate.setConfirmCallback(new RabbitTemplate.ConfirmCallback() {
            @Override
            public void confirm(CorrelationData correlationData, boolean ack, String cause) {
                if(ack){
                    log.info("消息发送成功:correlationData({}),ack({}),cause({})",correlationData,ack,cause);
                }else{
                    log.info("消息发送失败:correlationData({}),ack({}),cause({})",correlationData,ack,cause);
                }
            }
        });
        rabbitTemplate.setReturnCallback(new RabbitTemplate.ReturnCallback() {
            @Override
            public void returnedMessage(Message message, int replyCode, String replyText, String exchange, String routingKey) {
                log.info("消息丢失:exchange({}),route({}),replyCode({}),replyText({}),message:{}",exchange,routingKey,replyCode,replyText,message);
            }
        });
        return rabbitTemplate;
    }

    public String getCustomerTopicExchange() {
        return CustomerTopicExchange;
    }

    public String getCustomerQueue() {
        return CustomerQueue;
    }

    public String getCustomerRoutingKey() {
        return CustomerRoutingKey;
    }

    public String getServerTopicExchange() {
        return ServerTopicExchange;
    }

    public String getServerQueue() {
        return ServerQueue;
    }

    public String getServerRoutingKey() {
        return ServerRoutingKey;
    }
}