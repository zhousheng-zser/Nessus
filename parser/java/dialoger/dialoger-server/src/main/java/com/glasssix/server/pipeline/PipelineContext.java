package com.glasssix.server.pipeline;

import com.glasssix.common.util.ApplicationConstants;
import com.glasssix.server.rabbitmq.RabbitMQSender;
import com.google.gson.Gson;
import com.google.gson.JsonObject;
import lombok.extern.slf4j.Slf4j;
import org.springframework.amqp.core.Message;
import org.springframework.amqp.core.MessageProperties;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.context.annotation.Scope;
import org.springframework.scheduling.annotation.Async;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

@Slf4j
@Component
@Scope("prototype")
public class PipelineContext {
    private List<List<ValveHandlerCommon>> pipeline = new ArrayList<>();


    @Autowired
    private PipelineRegistListCache pipelineRegistListCache;

    private List<PipelineRegistEntry> lineNodes = null;

    private Message message;

    @Autowired
    private Gson gson;



    @Value("${rabbit.customer.queue.routingKeyPrefix}")
    private String routingKeyPrefix;

    @Autowired
    private RabbitMQSender rabbitMQSender;

    public PipelineContext() {
        lineNodes = pipelineRegistListCache.getLineNodes();
        Collections.sort(lineNodes, new Comparator<PipelineRegistEntry>() {
            @Override
            public int compare(PipelineRegistEntry o1, PipelineRegistEntry o2) {
                return o1.getOrder()-o2.getOrder();
            }
        });
    }

    @Async("taskExecutor")
    public void runPipeline(){
        if(message == null || message.getMessageProperties() ==null){
            return;
        }
        MessageProperties messageProperties = message.getMessageProperties();
        String receivedRoutingKey = messageProperties.getReceivedRoutingKey();
        String correlationDate = (String) messageProperties.getHeaders().get("spring_returned_message_correlation");
        String msg = new String(message.getBody());
        JsonObject jsonObject =gson.fromJson(msg,JsonObject.class);
        String endPoint = jsonObject.get("endPoint").getAsString();
        String result = configValveList(endPoint);
        if(ApplicationConstants.OK_STATIC.equals(result)) {
            result = executorPipeline(receivedRoutingKey, correlationDate, jsonObject);
        }
        rabbitMQSender.serverSend(receivedRoutingKey,result,correlationDate);

    }

    private String executorPipeline(String receivedRoutingKey, String correlationDate, JsonObject jsonObject) {
        String eventId = jsonObject.get("event_id").getAsString();
        for(int i=0;i<pipeline.size();i++){
            List<ValveHandlerCommon> valveHandlers = pipeline.get(i);
            int max = -1;
            ValveHandlerCommon valveHandlerCommon = null;
            for(ValveHandlerCommon v:valveHandlers){
                int estimate = v.estimate();
                if(estimate>max){
                    max = estimate;
                    valveHandlerCommon = v;
                }
            }
            String handlerResult = valveHandlerCommon.handler(receivedRoutingKey,correlationDate,jsonObject);
            jsonObject = gson.fromJson(handlerResult,JsonObject.class);
        }
        jsonObject.addProperty("event_id",eventId);
        return gson.toJson(jsonObject);
    }



    private String configValveList(String endPoint) {
        StringBuffer resultBuffer = new StringBuffer("endPoint is not exist!");
        for(PipelineRegistEntry p: lineNodes){
            ArrayList<ValveHandlerCommon> valveHandlers = new ArrayList<>();
            pipeline.add(valveHandlers);
            List<ValveConfigEntry> valves = p.getValves();
            for(ValveConfigEntry v: valves){
                String valveClass = v.getValveClass();
                try {
                    Class<?> objectClass = Class.forName("com.glasssix.server.pipeline." + valveClass);
                    ValveHandlerCommon value = (ValveHandlerCommon) objectClass.newInstance();
                    value.setInstanceTopic(v.getInstanceTopic());
                    valveHandlers.add(value);
                } catch (ClassNotFoundException e) {
                    log.error("valve class {} is not exist!",valveClass);
                    e.printStackTrace();
                } catch (IllegalAccessException e) {
                    e.printStackTrace();
                } catch (InstantiationException e) {
                    e.printStackTrace();
                }
            }

            if(p.getName().equals(endPoint)){
                resultBuffer = new StringBuffer(ApplicationConstants.OK_STATIC);
                break;
            }
        }
        return new String(resultBuffer);
    }

    public Message getMessage() {
        return message;
    }

    public void setMessage(Message message) {
        this.message = message;
    }
}
