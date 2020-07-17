package com.glasssix.server.pipeline;

import com.glasssix.algorithm.AlgorithmFactory;
import com.glasssix.common.util.SpringUtil;
import com.google.gson.Gson;
import com.google.gson.JsonObject;
import org.springframework.beans.factory.annotation.Autowired;

import java.util.List;

public abstract class ValveHandlerCommon {
    @Autowired
    private AlgorithmFactory algorithmFactory;

    @Autowired
    private Gson gson;
    private String instanceTopic;

    public String handler(String receivedRoutingKey, String correlationDate, JsonObject jsonObject){
        String instanceGuid = getInstanceGuid(receivedRoutingKey,instanceTopic.split("\\.")[0],jsonObject.get("device").getAsInt());
        jsonObject.addProperty("instance_guid",instanceGuid);
        return AlgorithmFactory.getPARSER().parse(instanceTopic,gson.toJson(jsonObject));
    }
    public abstract int estimate();

    public String getInstanceGuid(String receivedRoutingKey,String instanceName,int device) {
        String guuidKey = AlgorithmFactory.getGuuidKey(receivedRoutingKey,device)+"."+instanceName;
        List<String> consumerGuuidList = algorithmFactory.getConsumerGuuidList(guuidKey);
        if(consumerGuuidList == null || consumerGuuidList.size() == 0){
            createInstance(instanceName);
            consumerGuuidList = algorithmFactory.getConsumerGuuidList(guuidKey);
        }
        String guuid = null;
        synchronized (Object.class){
            guuid = consumerGuuidList.remove(0);
            consumerGuuidList.add(guuid);
        }
        return guuid;
    }

    public abstract void createInstance(String instanceName);

    public String getInstanceTopic() {
        return instanceTopic;
    }

    public void setInstanceTopic(String instanceTopic) {
        this.instanceTopic = instanceTopic;
    }
}
