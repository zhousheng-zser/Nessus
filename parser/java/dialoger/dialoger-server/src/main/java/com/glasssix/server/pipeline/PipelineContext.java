package com.glasssix.server.pipeline;

import com.glasssix.common.util.ApplicationConstants;
import com.glasssix.server.rabbitmq.RabbitMQSender;
import com.google.gson.Gson;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import lombok.extern.slf4j.Slf4j;
import org.springframework.amqp.core.Message;
import org.springframework.amqp.core.MessageProperties;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.scheduling.annotation.Async;
import org.springframework.stereotype.Component;

import java.util.*;

@Slf4j
@Component
public class PipelineContext {
    @Autowired
    private Gson gson;
    private List<PipelineRegistEntry> lineNodes = null;

    private Message message;
    private List<List<ValveHandlerCommon>> pipeline;


    @Value("${rabbit.customer.queue.routingKeyPrefix}")
    private String routingKeyPrefix;

    @Autowired
    private RabbitMQSender rabbitMQSender;

    @Autowired
    public PipelineContext(PipelineRegistListCache pipelineRegistListCache) {
        lineNodes = pipelineRegistListCache.getLineNodes();
        Collections.sort(lineNodes, new Comparator<PipelineRegistEntry>() {
            @Override
            public int compare(PipelineRegistEntry o1, PipelineRegistEntry o2) {
                return o1.getOrder() - o2.getOrder();
            }
        });
    }

    @Async("taskExecutor")
    public void runPipeline() {
        if (message == null || message.getMessageProperties() == null) {
            return;
        }
        MessageProperties messageProperties = message.getMessageProperties();
        String receivedRoutingKey = messageProperties.getReceivedRoutingKey();
        String correlationDate = (String) messageProperties.getHeaders().get("spring_returned_message_correlation");
        String msg = new String(message.getBody());
        JsonObject jsonObject = gson.fromJson(msg, JsonObject.class);
        String endPoint = jsonObject.get("endPoint").getAsString();
        String result = configValveList(endPoint);
        if (ApplicationConstants.OK_STATIC.equals(result)) {
            result = executorPipeline(receivedRoutingKey, correlationDate, jsonObject);
        }
        rabbitMQSender.serverSend(receivedRoutingKey, result, correlationDate);

    }

    private String executorPipeline(String receivedRoutingKey, String correlationDate, JsonObject jsonObject) {
        String eventId = jsonObject.get("event_id").getAsString();
        boolean flag = true;
        String handlerResult = null;
        JsonObject oldJsonObject = null;
        String currentInstanceTopic = null;
        for (int i = 0; i < pipeline.size(); i++) {
            List<ValveHandlerCommon> valveHandlers = pipeline.get(i);
            int max = -1;
            ValveHandlerCommon valveHandlerCommon = null;
            for (ValveHandlerCommon v : valveHandlers) {
                int estimate = v.estimate();
                if (estimate > max) {
                    max = estimate;
                    valveHandlerCommon = v;
                }
            }
            valveHandlerCommon.addPropertyAsInput(jsonObject,oldJsonObject);
            currentInstanceTopic = valveHandlerCommon.getInstanceTopic();
            handlerResult = valveHandlerCommon.handler(receivedRoutingKey, correlationDate, jsonObject);
            oldJsonObject = jsonObject;
            if (handlerResult != null) {
                jsonObject = gson.fromJson(handlerResult, JsonObject.class);
                if(ApplicationConstants.OK_STATIC.equals(jsonObject.get("status").getAsString())){
                    continue;
                }
            }
            flag = false;
            break;

        }
        if (!flag && handlerResult == null) {
            jsonObject = new JsonObject();
            jsonObject.addProperty("status", currentInstanceTopic+" error null");
        }
        jsonObject.addProperty("event_id", eventId);
        return gson.toJson(jsonObject);
    }


    private String configValveList(String endPoint) {
        StringBuffer resultBuffer = new StringBuffer("endPoint is not exist!");
        pipeline = new ArrayList<>();
        for (PipelineRegistEntry p : lineNodes) {
            ArrayList<ValveHandlerCommon> valveHandlers = new ArrayList<>();
            pipeline.add(valveHandlers);
            List<ValveConfigEntry> valves = p.getValves();
            for (ValveConfigEntry v : valves) {
                String valveClass = v.getValveClass();
                try {
                    Class<?> objectClass = Class.forName("com.glasssix.server.pipeline." + valveClass);
                    ValveHandlerCommon value = (ValveHandlerCommon) objectClass.newInstance();
                    value.setInstanceTopic(v.getInstanceTopic());
                    valveHandlers.add(value);
                } catch (ClassNotFoundException e) {
                    log.error("valve class {} is not exist!", valveClass);
                    e.printStackTrace();
                } catch (IllegalAccessException e) {
                    e.printStackTrace();
                } catch (InstantiationException e) {
                    e.printStackTrace();
                }
            }

            if (p.getName().equals(endPoint)) {
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
