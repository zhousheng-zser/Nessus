package com.glasssix.server.pipeline.irisviel;

import com.glasssix.common.util.ApplicationConstants;
import com.glasssix.protocol.result.NewResultProtocol;
import com.google.gson.JsonObject;
import org.springframework.context.annotation.Scope;
import org.springframework.stereotype.Component;

@Component
@Scope("prototype")
public class PersonDBNewValve extends PersonDBValveCommon {

    @Override
    public String handler(String receivedRoutingKey, String correlationDate, JsonObject jsonObject) {
        String result = super.handler(receivedRoutingKey,correlationDate,jsonObject);
        NewResultProtocol newResultProtocol = gson.fromJson(result, NewResultProtocol.class);
        if(ApplicationConstants.OK_STATIC.equals(newResultProtocol.getStatus())){
            String instanceGuid = newResultProtocol.getInstanceGuid();
            String uuid = instanceMapFileOption.getUUId();
            //irisvielMapFileOption.add(instanceGuid,jsonObject.get("working_directory").getAsString());
            instanceFileOption.add(uuid,gson.toJson(jsonObject));
            instanceMapFileOption.add(uuid,instanceGuid);
            algorithmFactory.setConsumerGuuid(getGuuidKey(receivedRoutingKey,-1),instanceGuid);
            newResultProtocol.setInstanceGuid(uuid);
            result = gson.toJson(newResultProtocol);
        }
        return result;
    }



}
