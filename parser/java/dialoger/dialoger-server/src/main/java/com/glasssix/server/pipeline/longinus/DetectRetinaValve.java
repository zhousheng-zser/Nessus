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
import org.springframework.context.annotation.Scope;
import org.springframework.stereotype.Component;
import org.springframework.util.CollectionUtils;

import java.util.List;
import java.util.concurrent.locks.ReentrantLock;

@Slf4j
@Component
@Scope("prototype")
public class DetectRetinaValve extends ValveHandlerCommon {

    @Override
    public int estimate() {
        return 0;
    }

    @Autowired
    private AlgorithmFactory algorithmFactory;

    @Override
    public String getInstanceGuid() {
        String instanceGuid = null;
        String guuidKey = getGuuidKey(receivedRoutingKey, device);
        synchronized (algorithmFactory) {
            List<String> consumerGuuidList = algorithmFactory.getConsumerGuuidList(guuidKey);
            if (CollectionUtils.isEmpty(consumerGuuidList)) {
                instanceGuid = createInstance();
                algorithmFactory.setConsumerGuuid(guuidKey, instanceGuid);
            }else {
                instanceGuid = consumerGuuidList.remove(0);
                consumerGuuidList.add(instanceGuid);
            }
        }

        return instanceGuid;
    }

    private String getGuuidKey(String receivedRoutingKey, int device) {
        String guuidKey = AlgorithmFactory.getGuuidKey(receivedRoutingKey, device);
        return guuidKey+".Longinus";
    }

    @Override
    public String createInstance() {
        NewProtocol newProtocol = new NewProtocol();
        newProtocol.setDevice(-1);
        String jsonStr = AlgorithmFactory.getPARSER().parse("longinus.new",gson.toJson(newProtocol));
        NewResultProtocol newResultProtocol = gson.fromJson(jsonStr, NewResultProtocol.class);
        if(ApplicationConstants.OK_STATIC.equals(newResultProtocol.getStatus())){
            return newResultProtocol.getInstanceGuid();
        }
        log.error("longinus.new return error: {}",newResultProtocol==null? null:newResultProtocol.getStatus());
        return null;
    }
}
