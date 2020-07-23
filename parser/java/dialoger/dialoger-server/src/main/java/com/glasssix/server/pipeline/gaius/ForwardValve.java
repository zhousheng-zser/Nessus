package com.glasssix.server.pipeline.gaius;

import com.glasssix.algorithm.AlgorithmFactory;
import com.glasssix.common.util.ApplicationConstants;
import com.glasssix.protocol.NewProtocol;
import com.glasssix.protocol.result.NewResultProtocol;
import com.glasssix.server.pipeline.ThreadLocalResource;
import com.glasssix.server.pipeline.ValveHandlerCommon;
import com.google.gson.JsonObject;
import lombok.extern.slf4j.Slf4j;

@Slf4j
public class ForwardValve extends ValveHandlerCommon {
    @Override
    public int estimate() {
        return 0;
    }

    @Override
    public String getInstanceGuid() {
        String gasiusInstanceGuid = ThreadLocalResource.gaiusInstance.get();
        if(gasiusInstanceGuid == null){
            gasiusInstanceGuid = createInstance();
        }
        return gasiusInstanceGuid;
    }

    @Override
    public String createInstance() {
        NewProtocol newProtocol = new NewProtocol();
        newProtocol.setDevice(-1);
        String jsonStr = AlgorithmFactory.getPARSER().parse("Gaius.new", gson.toJson(newProtocol));
        NewResultProtocol newResultProtocol = gson.fromJson(jsonStr, NewResultProtocol.class);
        if (ApplicationConstants.OK_STATIC.equals(newResultProtocol.getStatus())) {
            ThreadLocalResource.gaiusInstance.set(newResultProtocol.getInstanceGuid());
            return newResultProtocol.getInstanceGuid();
        }
        log.error("Gaius.new return error: {}", newResultProtocol == null ? null : newResultProtocol.getStatus());
        return null;
    }

    @Override
    public void addPropertyAsInput(JsonObject jsonObject, JsonObject oldJsonObject) {
        super.addPropertyAsInput(jsonObject,oldJsonObject);
        jsonObject.add("aligned_images",jsonObject.get("aligned_faces"));
        jsonObject.add("order",oldJsonObject.get("order"));
        jsonObject.remove("aligned_faces");
        jsonObject.remove("image");
    }
}
