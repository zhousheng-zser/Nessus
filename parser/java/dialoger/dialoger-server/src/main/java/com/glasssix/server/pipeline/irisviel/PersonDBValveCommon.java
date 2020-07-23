package com.glasssix.server.pipeline.irisviel;

import com.glasssix.algorithm.AlgorithmFactory;
import com.glasssix.server.pipeline.ThreadLocalResource;
import com.glasssix.server.pipeline.ValveHandlerCommon;
import com.google.gson.JsonObject;

public class PersonDBValveCommon extends ValveHandlerCommon {

    private String irisvielInstanceGuid;

    @Override
    public String handler(String receivedRoutingKey, String correlationDate, JsonObject jsonObject) {
        return AlgorithmFactory.getPARSER().parse(super.getInstanceTopic(), gson.toJson(jsonObject));
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
