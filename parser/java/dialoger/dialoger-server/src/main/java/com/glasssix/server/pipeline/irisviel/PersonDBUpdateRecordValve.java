package com.glasssix.server.pipeline.irisviel;

import com.google.gson.JsonObject;
import org.springframework.context.annotation.Scope;
import org.springframework.stereotype.Component;

@Component
@Scope("prototype")
public class PersonDBUpdateRecordValve extends PersonDBValveCommon {
    @Override
    public void addPropertyAsInput(JsonObject jsonObject, JsonObject oldJsonObject) {
        //super.addPropertyAsInput(jsonObject,oldJsonObject);
        JsonObject tempJsonObject = new JsonObject();
        tempJsonObject.add("feature",jsonObject.getAsJsonArray("features").get(0).getAsJsonObject().getAsJsonArray("feature"));
        tempJsonObject.add("key",oldJsonObject.get("key"));
        jsonObject.add("data",tempJsonObject);
        jsonObject.addProperty("instance_guid",super.getIrisvielInstanceGuid());
    }
}
