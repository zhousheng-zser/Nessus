package com.glasssix.server.threadPool;

import com.glasssix.server.protocol.ProtocolCache;
import com.glasssix.server.protocol.ProtocolInterface;
import com.glasssix.server.rabbitmq.RabbitMQSender;
import com.google.gson.Gson;
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
import java.util.UUID;

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


    @Async("taskExecutor")
    public void runJNITask(){
        if(message == null || message.getMessageProperties() ==null){
            return;
        }
        MessageProperties messageProperties = message.getMessageProperties();
        String receivedRoutingKey = messageProperties.getReceivedRoutingKey();
        String correlationDate = (String) messageProperties.getHeaders().get("spring_returned_message_correlation");

        String algorithmName = receivedRoutingKey.split("\\.")[5];
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
            if(!(algorithmObject instanceof ProtocolInterface)){
                log.error("correlationData({}) call algorithm({}) is not protocol",correlationDate,algorithmName);
                return;
            }
            ProtocolInterface protocolInterface = (ProtocolInterface) algorithmObject;
            protocolInterface.protocolProcess(receivedRoutingKey,correlationDate);

            String params = transformToCallJNIParam(receivedRoutingKey, correlationDate);
            String result = callJNI(params);//JNI调用
            String backResult = transformToBackStr(receivedRoutingKey, correlationDate, result);
            String backRoutingKey = getBackRoutingKey(receivedRoutingKey);
            log.info(correlationDate);
            rabbitMQSender.serverSend(backRoutingKey,backResult,correlationDate);
        } catch (ClassNotFoundException e) {
            e.printStackTrace();
        }
    }

    private String callJNI(String params) {
        log.info("JNI Interface call，params:"+params);
        return "wait"+ UUID.randomUUID();
    }

    private String getBackRoutingKey(String receivedRoutingKey) {
        String[] keyArray = receivedRoutingKey.split("\\.");
        String resultRoutingKey = serverRoutingKeyPrefix + keyArray[3]+"."+keyArray[4] + serverRoutingKeySuffix;
        return resultRoutingKey;

    }

    /**
     * 在调用JNI之前，通过该方法将信息封装成JNI接口需要的字符串
     * @param receivedRoutingKey
     * @param correlationDate
     * @return
     */
    public String transformToCallJNIParam(String receivedRoutingKey, String correlationDate) {
        return "receivedRoutingKey:"+receivedRoutingKey+",correlationDate:"+correlationDate;
    }

    /**
     * 在调用JNI后，将JNI返回的结果封装成返回给客户的字符串
     * @param receivedRoutingKey
     * @param correlationDate
     * @return
     */
    public String transformToBackStr(String receivedRoutingKey, String correlationDate, String JNIResult) {
        return "receivedRoutingKey:"+receivedRoutingKey+",correlationDate:"+correlationDate+",JNIResult"+JNIResult;
    }


    public Message getMessage() {
        return message;
    }

    public void setMessage(Message message) {
        this.message = message;
    }
}
