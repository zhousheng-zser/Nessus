package com.glasssix.server.rabbitmq;

import com.glasssix.server.pipeline.PipelineContext;
import com.glasssix.server.threadPool.TaskContext;
import lombok.extern.slf4j.Slf4j;
import org.springframework.amqp.core.Message;
import org.springframework.amqp.rabbit.annotation.*;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.context.ApplicationContext;
import org.springframework.stereotype.Component;

@Slf4j
@Component
public class RabbitMQReceiver {

    @Autowired
    private ApplicationContext context;


    @RabbitListener(bindings = {@QueueBinding(value = @Queue(value = "${rabbit.customer.queue.name}",
            durable = "true",arguments = {@Argument(name="x-message-ttl",value = "${rabbit.customer.queue.timeToLive}",type = "java.lang.Integer")}),
                                    exchange = @Exchange(value = "${rabbit.customer.topicExchange.name}", type = "topic"),
                                    key = "${rabbit.customer.queue.routingKey}")})
    @RabbitHandler
    public void process(Message message) {
        String receivedRoutingKey = message.getMessageProperties().getReceivedRoutingKey();
        if(receivedRoutingKey.contains("pipeline")){
            PipelineContext pipelineContext = context.getBean(PipelineContext.class);
            pipelineContext.setMessage(message);
            pipelineContext.runPipeline();
        }else{
            TaskContext taskContext = context.getBean(TaskContext.class);
            taskContext.setMessage(message);
            taskContext.runJNITask();
        }


    }

}
