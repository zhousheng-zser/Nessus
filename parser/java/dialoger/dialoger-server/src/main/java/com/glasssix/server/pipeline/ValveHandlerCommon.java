package com.glasssix.server.pipeline;

import com.glasssix.algorithm.AlgorithmFactory;
import com.google.gson.Gson;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import org.springframework.beans.factory.annotation.Autowired;

import java.util.Iterator;
import java.util.Map;

public abstract class ValveHandlerCommon {

    protected Gson gson;
    private String instanceTopic;

    public ValveHandlerCommon(){
        gson = new Gson();
    }

    public String handler(String receivedRoutingKey, String correlationDate, JsonObject jsonObject) {
        String instanceGuid = getInstanceGuid();
        if (instanceGuid == null) {
            return null;
        }
        jsonObject.addProperty("instance_guid", instanceGuid);
        return AlgorithmFactory.getPARSER().parse(instanceTopic, gson.toJson(jsonObject));
    }

    public void addPropertyAsInput(JsonObject jsonObject, JsonObject oldJsonObject) {
        if(oldJsonObject == null){
            return;
        }
        Iterator<Map.Entry<String, JsonElement>> iterator = oldJsonObject.entrySet().iterator();
        while (iterator.hasNext()){
            Map.Entry<String, JsonElement> next = iterator.next();
            String key = next.getKey();
            if((jsonObject.get(key)==null && !"image".equals(key))||"instance_guid".equals(key)){
                jsonObject.add(key,next.getValue());
            }
        }
    }

    public abstract int estimate();

    public abstract String getInstanceGuid();

    public abstract String createInstance();

    public String getInstanceTopic() {
        return instanceTopic;
    }

    public void setInstanceTopic(String instanceTopic) {
        this.instanceTopic = instanceTopic;
    }
}
