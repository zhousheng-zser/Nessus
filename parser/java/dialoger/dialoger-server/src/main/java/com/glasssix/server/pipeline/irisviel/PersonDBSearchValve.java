package com.glasssix.server.pipeline.irisviel;

import com.google.gson.JsonObject;
import org.springframework.context.annotation.Scope;
import org.springframework.stereotype.Component;

@Component
@Scope("prototype")
public class PersonDBSearchValve extends PersonDBValveCommon {

    @Override
    public void addPropertyAsInput(JsonObject jsonObject, JsonObject oldJsonObject) {
        //super.addPropertyAsInput(jsonObject,oldJsonObject);
        JsonObject tempJsonObject = new JsonObject();
        jsonObject.add("feature",jsonObject.getAsJsonArray("features").get(0).getAsJsonObject().getAsJsonArray("feature"));
        jsonObject.addProperty("instance_guid",super.getIrisvielInstanceGuid());
        jsonObject.add("top",oldJsonObject.get("top"));
    }
}
