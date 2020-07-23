package com.glasssix.server.pipeline.irisviel;

import com.google.gson.JsonObject;

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
