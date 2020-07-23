package com.glasssix.server.pipeline.cassius;

import com.glasssix.algorithm.AlgorithmFactory;
import com.glasssix.common.util.ApplicationConstants;
import com.glasssix.protocol.NewProtocol;
import com.glasssix.protocol.result.NewResultProtocol;
import com.glasssix.server.pipeline.ThreadLocalResource;
import com.glasssix.server.pipeline.ValveHandlerCommon;
import com.google.gson.JsonObject;
import lombok.extern.slf4j.Slf4j;
import org.springframework.stereotype.Component;

@Slf4j
public class ForwardValve extends ValveHandlerCommon {
    @Override
    public int estimate() {
        return 1;
    }

    @Override
    public String getInstanceGuid() {
        String gasiusInstanceGuid = ThreadLocalResource.cassiusInstance.get();
        if(gasiusInstanceGuid == null){
            gasiusInstanceGuid = createInstance();
            ThreadLocalResource.cassiusInstance.set(gasiusInstanceGuid);
        }
        return gasiusInstanceGuid;
    }

    @Override
    public String createInstance() {
        NewProtocol newProtocol = new NewProtocol();
        newProtocol.setDevice(-1);
        String jsonStr = AlgorithmFactory.getPARSER().parse("Cassius.new", gson.toJson(newProtocol));
        NewResultProtocol newResultProtocol = gson.fromJson(jsonStr, NewResultProtocol.class);
        if (ApplicationConstants.OK_STATIC.equals(newResultProtocol.getStatus())) {
            ThreadLocalResource.cassiusInstance.set(newResultProtocol.getInstanceGuid());
            return newResultProtocol.getInstanceGuid();
        }
        log.error("Cassius.new return error: {}", newResultProtocol == null ? null : newResultProtocol.getStatus());
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
