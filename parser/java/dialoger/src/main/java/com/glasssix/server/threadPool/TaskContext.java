package com.glasssix.server.threadPool;

import com.glasssix.server.algorithm.AlgorithmFactory;
import com.glasssix.parser.Parser;
import com.glasssix.server.protocol.ProtocolCache;
import com.glasssix.server.protocol.ProtocolCommon;
import com.glasssix.server.protocol.longinus.DetectExProtocol;
import com.glasssix.server.protocol.result.NewResultProtocol;
import com.glasssix.server.rabbitmq.RabbitMQSender;
import com.glasssix.server.util.ApplicationConstants;
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
import org.springframework.util.StringUtils;

import java.nio.charset.StandardCharsets;
import java.util.Base64;

@Slf4j
@Component
@Scope("prototype")
public class TaskContext {

    @Autowired
    private RabbitMQSender rabbitMQSender;

    @Autowired
    private Gson gson;

    @Value("${rabbit.server.queue.routingKeyPrefix}")
    private String serverRoutingKeyPrefix;

    @Value("${rabbit.server.queue.routingKeySuffix}")
    private String serverRoutingKeySuffix;

    private Message message;

    private Parser parser;

    private AlgorithmFactory algorithmFactory;

    @Autowired
    TaskContext(AlgorithmFactory algorithmFactory){
        parser = algorithmFactory.getPARSER();
        this.algorithmFactory = algorithmFactory;
    }


    @Async("taskExecutor")
    public void runJNITask(){
        if(message == null || message.getMessageProperties() ==null){
            return;
        }
        MessageProperties messageProperties = message.getMessageProperties();
        String receivedRoutingKey = messageProperties.getReceivedRoutingKey();
        String correlationDate = (String) messageProperties.getHeaders().get("spring_returned_message_correlation");
        String[] routingItems = receivedRoutingKey.split("\\.");
        String algorithmName = routingItems[4]+"."+routingItems[5];
        String entryName = ProtocolCache.getEntryName(algorithmName);
        if(StringUtils.isEmpty(entryName)){
            log.error("correlationData({}) call algorithm({}) is not register",correlationDate,algorithmName);
            return;
        }
        String msg = new String(message.getBody(), StandardCharsets.UTF_8);
        log.info("receive correlationData({})",correlationDate);

        try {
            Class<?> algorithmClass = Class.forName("com.glasssix.server.protocol." + entryName);
            Object algorithmObject = gson.fromJson(msg, algorithmClass);
            if(!(algorithmObject instanceof ProtocolCommon)){
                log.error("correlationData({}) call algorithm({}) is not protocol",correlationDate,algorithmName);
                return;
            }
            ProtocolCommon protocolCommon = (ProtocolCommon) algorithmObject;
            String processResult = protocolCommon.protocolProcess(receivedRoutingKey,correlationDate,protocolCommon);
            String backResult = null;
            if(ApplicationConstants.OK_STATIC.equals(processResult)){
                String params = transformToCallJNIParam(receivedRoutingKey, correlationDate, protocolCommon);
                String result = parser.parse(algorithmName,params);
                resultProcess(result,receivedRoutingKey,correlationDate,protocolCommon);
                backResult = transformToBackStr(receivedRoutingKey, correlationDate, result,protocolCommon.eventId);
            }else{
                NewResultProtocol newResultProtocol = new NewResultProtocol();
                newResultProtocol.setStatus(processResult);
                newResultProtocol.setEventId(protocolCommon.getEventId());
                backResult = gson.toJson(newResultProtocol);
            }
            String backRoutingKey = getBackRoutingKey(receivedRoutingKey);
            rabbitMQSender.serverSend(backRoutingKey,backResult,correlationDate);

        } catch (ClassNotFoundException e) {
            e.printStackTrace();
        }
    }

    private void resultProcess(String result, String receivedRoutingKey, String correlationDate,
                               ProtocolCommon protocolCommon) {

        JsonObject jsonObject = gson.fromJson(result, JsonObject.class);
        String status = jsonObject.get("status").getAsString();
        log.info("JNI receive: "+result);
        if(ApplicationConstants.OK_STATIC.equals(status)){
            String[] routingItems = receivedRoutingKey.split("\\.");
            if("new".equals(routingItems[5])){
                addGuuid(algorithmFactory.getGuuidKey(receivedRoutingKey,protocolCommon),
                        jsonObject.get("instance_guid").getAsString());
            }
            if("delete".equals(routingItems[5])){
                deleteGuuid(algorithmFactory.getGuuidKey(receivedRoutingKey,protocolCommon),
                        protocolCommon);
            }
        }else {
            log.error("correlationDate({}) back message:{}",correlationDate,result);
        }
    }


    public void addGuuid(String key,String guuid){
        if(!StringUtils.isEmpty(guuid)){
            algorithmFactory.setConsumerGuuid(key,guuid);
        }

    }

    public void deleteGuuid(String key,ProtocolCommon protocolCommon){
        algorithmFactory.deleteConsumerGuuid(key,protocolCommon.instanceGuid);
    }

    /**
     * 预留该方法，之后如果要做返回的topic和进来的topic不一致就在这里做
     * @param receivedRoutingKey
     * @return
     */
    private String getBackRoutingKey(String receivedRoutingKey) {
        return receivedRoutingKey;
    }

    /**
     * 在调用JNI之前，通过该方法将信息封装成JNI接口需要的字符串
     * @param receivedRoutingKey
     * @param correlationDate
     * @return
     */
    public String transformToCallJNIParam(String receivedRoutingKey, String correlationDate, ProtocolCommon protocolCommon) {
        log.debug("CALL JNI JSON: "+gson.toJson(protocolCommon));
        return gson.toJson(protocolCommon);
    }

    /**
     * 在调用JNI后，将JNI返回的结果封装成返回给客户的字符串
     * @param receivedRoutingKey
     * @param correlationDate
     * @return
     */
    public String transformToBackStr(String receivedRoutingKey, String correlationDate, String JNIResult,String eventId) {
        return JNIResult.substring(0,JNIResult.length()-2)+",\"event_id\":"+eventId+"}";
    }


    public Message getMessage() {
        return message;
    }

    public void setMessage(Message message) {
        this.message = message;
    }
}
