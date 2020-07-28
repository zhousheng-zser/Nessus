package com.glasssix.server.pipeline;

import com.glasssix.common.util.ApplicationConstants;
import com.glasssix.common.util.SpringUtil;
import com.glasssix.protocol.result.NewResultProtocol;
import com.glasssix.server.pipeline.irisviel.PersonDBValveCommon;
import com.glasssix.server.rabbitmq.RabbitMQSender;
import com.google.gson.Gson;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import lombok.extern.slf4j.Slf4j;
import org.springframework.amqp.core.Message;
import org.springframework.amqp.core.MessageProperties;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.context.annotation.Scope;
import org.springframework.scheduling.annotation.Async;
import org.springframework.stereotype.Component;

import javax.annotation.PostConstruct;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

@Slf4j
@Component
@Scope("prototype")
public class PipelineContext {
    @Autowired
    private Gson gson;
    private List<PipelineRegistEntry> lineNodes = null;
    private List<ValveConfigEntry> signleNodes = null;
    private List<EndPointEnum> onlyNeedSignleNode = new ArrayList<>();
    private List<EndPointEnum> onlyNeedLineNode = new ArrayList<>();
    private List<EndPointEnum> needLineAndSignleNode = new ArrayList<>();

    private Message message;
    private List<List<ValveHandlerCommon>> pipeline;


    @Value("${rabbit.customer.queue.routingKeyPrefix}")
    private String routingKeyPrefix;

    @Autowired
    private RabbitMQSender rabbitMQSender;

    @Autowired
    public PipelineContext(PipelineRegistListCache pipelineRegistListCache) {
        lineNodes = pipelineRegistListCache.getLineNodes();
        signleNodes = pipelineRegistListCache.getSignleNodes();
    }

    @PostConstruct
    public void configInit(){
        Collections.sort(lineNodes, new Comparator<PipelineRegistEntry>() {
            @Override
            public int compare(PipelineRegistEntry o1, PipelineRegistEntry o2) {
                return o1.getOrder() - o2.getOrder();
            }
        });
        onlyNeedLineNode.add(EndPointEnum.DETECT);
        onlyNeedLineNode.add(EndPointEnum.ALIGN);
        onlyNeedLineNode.add(EndPointEnum.FORWARD);

        onlyNeedSignleNode.add(EndPointEnum.ADDS);
        onlyNeedSignleNode.add(EndPointEnum.UPDATES);
        onlyNeedSignleNode.add(EndPointEnum.NEW);
        onlyNeedSignleNode.add(EndPointEnum.REMOVE);
        onlyNeedSignleNode.add(EndPointEnum.REMOVES);
        onlyNeedSignleNode.add(EndPointEnum.DELETE);
        onlyNeedSignleNode.add(EndPointEnum.LOAD);
        onlyNeedSignleNode.add(EndPointEnum.REMOVE_ALL);

        needLineAndSignleNode.add(EndPointEnum.ADD);
        needLineAndSignleNode.add(EndPointEnum.SEARCH);
        needLineAndSignleNode.add(EndPointEnum.UPDATE);
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
        String execResult = exec(receivedRoutingKey, correlationDate,jsonObject);
        rabbitMQSender.serverSend(receivedRoutingKey, execResult, correlationDate);

    }

    public String exec(String receivedRoutingKey,String correlationDate,JsonObject jsonObject){
        String endPoint = jsonObject.get("endPoint").getAsString();
        String event_id = jsonObject.get("event_id").getAsString();
        JsonElement instance_guid = jsonObject.get("instance_guid");
        EndPointEnum endPointEnum = EndPointEnum.valueOf(endPoint.toUpperCase());
        String result = selectNodes(endPointEnum,instance_guid== null? null:instance_guid.getAsString());
        if (ApplicationConstants.OK_STATIC.equals(result)) {
            result = executorPipeline(receivedRoutingKey, correlationDate, jsonObject);
        }else{
            NewResultProtocol newResultProtocol = new NewResultProtocol();
            newResultProtocol.setStatus(result);
            newResultProtocol.setEventId(event_id);
            result = gson.toJson(newResultProtocol);
        }
        return result;
    }

    private String selectNodes(EndPointEnum endPointEnum, String instanceGuid) {
        String result = null;
        if(endPointEnum == null){
            return "endPoint: "+endPointEnum.getEndPointName()+" is not exist!";
        }

        if(onlyNeedSignleNode.contains(endPointEnum)){
            result = configSignleNode(endPointEnum,instanceGuid);
        }else{
            if(onlyNeedLineNode.contains(endPointEnum)){
                result = configValveList(endPointEnum.getEndPointName());
            }else{
                result = configLineNodesAndSignleNode(endPointEnum,instanceGuid);
            }
        }

        return result;
    }

    private String configLineNodesAndSignleNode(EndPointEnum endPointEnum,String instanceGuid){
        String result = configValveList(EndPointEnum.FORWARD.getEndPointName());
        if (ApplicationConstants.OK_STATIC.equals(result)) {
            result = "endPoint "+endPointEnum.getEndPointName()+" is not exist!";
            for(ValveConfigEntry v:signleNodes){
                if(endPointEnum.getEndPointName().equals(v.getName())){
                    List<ValveHandlerCommon> valves = new ArrayList<>();
                    PersonDBValveCommon vc = (PersonDBValveCommon) createValveHandlerCommon(v.getValveClass(),v.getInstanceTopic());
                    vc.setIrisvielInstanceGuid(instanceGuid);
                    valves.add(vc);
                    pipeline.add(valves);
                    result = ApplicationConstants.OK_STATIC;
                    break;
                }
            }
        }
        return result;
    }

    private String configSignleNode(EndPointEnum endPointEnum,String instanceGuid){
        StringBuffer resultBuffer = new StringBuffer("endPoint ").append(endPointEnum.getEndPointName()).append(" is not exist!");
        pipeline = new ArrayList<>();
        for(ValveConfigEntry v:signleNodes){
            if(endPointEnum.getEndPointName().equals(v.getName())){
                List<ValveHandlerCommon> valves = new ArrayList<>();
                PersonDBValveCommon vc = (PersonDBValveCommon) createValveHandlerCommon(v.getValveClass(),v.getInstanceTopic());
                vc.setIrisvielInstanceGuid(instanceGuid);
                valves.add(vc);
                pipeline.add(valves);
                resultBuffer = new StringBuffer(ApplicationConstants.OK_STATIC);
                break;
            }
        }
        return new String(resultBuffer);
    }

    private String configValveList(String endPoint) {
        StringBuffer resultBuffer = new StringBuffer("endPoint ").append(endPoint).append(" is not exist!");
        pipeline = new ArrayList<>();
        for (PipelineRegistEntry p : lineNodes) {
            ArrayList<ValveHandlerCommon> valveHandlers = new ArrayList<>();
            pipeline.add(valveHandlers);
            List<ValveConfigEntry> valves = p.getValves();
            for (ValveConfigEntry v : valves) {
                String valveClass = v.getValveClass();
                ValveHandlerCommon valve = createValveHandlerCommon(valveClass,v.getInstanceTopic());
                valveHandlers.add(valve);
            }
            if (p.getName().equals(endPoint)) {
                resultBuffer = new StringBuffer(ApplicationConstants.OK_STATIC);
                break;
            }
        }
        return new String(resultBuffer);
    }

    private ValveHandlerCommon createValveHandlerCommon(String valveClass,String instanceTopic){
        ValveHandlerCommon valve = null;
        try {
            Class<?> objectClass = Class.forName("com.glasssix.server.pipeline." + valveClass);
            //valve = (ValveHandlerCommon) objectClass.newInstance();
            valve = (ValveHandlerCommon) SpringUtil.getApplicationContext().getBean(objectClass);
            valve.setInstanceTopic(instanceTopic);
        } catch (ClassNotFoundException e) {
            log.error("valve class {} is not exist!", valveClass);
            e.printStackTrace();
        }
        return valve;
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
            JsonElement device = jsonObject.get("device");
            if(device != null){
                valveHandlerCommon.setDevice(device.getAsInt());
            }
            valveHandlerCommon.setReceivedRoutingKey(receivedRoutingKey);
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

    public Message getMessage() {
        return message;
    }

    public void setMessage(Message message) {
        this.message = message;
    }
}
