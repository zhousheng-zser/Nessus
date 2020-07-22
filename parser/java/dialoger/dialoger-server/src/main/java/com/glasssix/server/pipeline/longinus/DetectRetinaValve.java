package com.glasssix.server.pipeline.longinus;

import com.glasssix.algorithm.AlgorithmFactory;
import com.glasssix.common.util.ApplicationConstants;
import com.glasssix.protocol.NewProtocol;
import com.glasssix.protocol.result.NewResultProtocol;
import com.glasssix.server.pipeline.ThreadLocalResource;
import com.glasssix.server.pipeline.ValveHandlerCommon;
import com.google.gson.Gson;
import lombok.extern.slf4j.Slf4j;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

@Slf4j
@Component
public class DetectRetinaValve extends ValveHandlerCommon {

    @Override
    public int estimate() {
        return 0;
    }

    @Override
    public String getInstanceGuid() {
        String instanceGuid = ThreadLocalResource.longinusInstance.get();
        if(instanceGuid == null){
            instanceGuid = createInstance();
        }
        return instanceGuid;
    }

    @Override
    public String createInstance() {
        NewProtocol newProtocol = new NewProtocol();
        newProtocol.setDevice(-1);
        String jsonStr = AlgorithmFactory.getPARSER().parse("longinus.new",gson.toJson(newProtocol));
        NewResultProtocol newResultProtocol = gson.fromJson(jsonStr, NewResultProtocol.class);
        if(ApplicationConstants.OK_STATIC.equals(newResultProtocol.getStatus())){
            ThreadLocalResource.longinusInstance.set(newResultProtocol.getInstanceGuid());
            return newResultProtocol.getInstanceGuid();
        }
        log.error("longinus.new return error: {}",newResultProtocol==null? null:newResultProtocol.getStatus());
        return null;
    }
}
