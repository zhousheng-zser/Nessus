package com.glasssix.server.pipeline.irisviel;

import com.glasssix.common.util.ApplicationConstants;
import com.glasssix.protocol.result.NewResultProtocol;
import com.glasssix.server.instancemap.ConsumerMapFileOption;
import com.glasssix.server.instancemap.InstanceFileOption;
import com.glasssix.server.instancemap.InstanceMapFileOption;
import com.google.gson.JsonObject;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.context.annotation.Scope;
import org.springframework.stereotype.Component;

@Component
@Scope("prototype")
public class PersonDBRemoveAllValve extends PersonDBValveCommon {
    @Autowired
    private ConsumerMapFileOption consumerMapFileOption;
    @Autowired
    private InstanceMapFileOption instanceMapFileOption;
    @Autowired
    private InstanceFileOption instanceFileOption;

    @Override
    public String handler(String receivedRoutingKey, String correlationDate, JsonObject jsonObject) {
        String uuid = getUuid();
        String result = super.handler(receivedRoutingKey,correlationDate,jsonObject);
        NewResultProtocol newResultProtocol = gson.fromJson(result, NewResultProtocol.class);
        if(ApplicationConstants.OK_STATIC.equals(newResultProtocol.getStatus())){
            instanceFileOption.delete(uuid);
            algorithmFactory.deleteConsumerGuuid(getGuuidKey(receivedRoutingKey,-1),instanceMapFileOption.getInstanceId(uuid));
            instanceMapFileOption.delete(uuid);
        }
        return result;
    }


}
