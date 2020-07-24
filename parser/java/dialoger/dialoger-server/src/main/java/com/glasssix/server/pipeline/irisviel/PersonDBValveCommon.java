package com.glasssix.server.pipeline.irisviel;

import com.glasssix.algorithm.AlgorithmFactory;
import com.glasssix.server.pipeline.ThreadLocalResource;
import com.glasssix.server.pipeline.ValveHandlerCommon;
import com.google.gson.JsonObject;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class PersonDBValveCommon extends ValveHandlerCommon {

    Logger log = LoggerFactory.getLogger(PersonDBValveCommon.class);
    private String irisvielInstanceGuid;

    @Override
    public String handler(String receivedRoutingKey, String correlationDate, JsonObject jsonObject) {
        log.info("CALL JNI with InstanceTopic:{} and param{}",super.getInstanceTopic(), gson.toJson(jsonObject));
        String result = AlgorithmFactory.getPARSER().parse(super.getInstanceTopic(), gson.toJson(jsonObject));
        log.info("JNI Receive: {}",result);
        return result;
    }

    @Override
    public int estimate() {
        return 0;
    }

    @Override
    public String getInstanceGuid() {
        return null;
    }

    @Override
    public String createInstance() {
        return null;
    }

    public String getIrisvielInstanceGuid() {
        return irisvielInstanceGuid;
    }

    public void setIrisvielInstanceGuid(String irisvielInstanceGuid) {
        this.irisvielInstanceGuid = irisvielInstanceGuid;
    }
}
