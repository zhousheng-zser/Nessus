package com.glasssix.server.pipeline.longinus;

import com.glasssix.algorithm.AlgorithmFactory;
import com.glasssix.common.util.ApplicationConstants;
import com.glasssix.protocol.NewProtocol;
import com.glasssix.protocol.result.NewResultProtocol;
import com.glasssix.server.pipeline.ThreadLocalResource;
import com.glasssix.server.pipeline.ValveHandlerCommon;
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
public class DetectExValve extends ValveHandlerCommon {

    @Override
    public int estimate() {
        return 1;
    }

    @Autowired
    private AlgorithmFactory algorithmFactory;

    @Override
    public String getInstanceGuid() {
        String instanceGuid = null;
        String guuidKey = AlgorithmFactory.getGuuidKey(receivedRoutingKey, device);
        synchronized (algorithmFactory) {
            List<String> consumerGuuidList = algorithmFactory.getConsumerGuuidList(guuidKey);
            if (CollectionUtils.isEmpty(consumerGuuidList)) {
                algorithmFactory.setConsumerGuuid(guuidKey, createInstance());
            }
            instanceGuid = consumerGuuidList.remove(0);
            consumerGuuidList.add(instanceGuid);
        }
        return instanceGuid;
    }

    @Override
    public String createInstance() {
        NewProtocol newProtocol = new NewProtocol();
        newProtocol.setDevice(-1);
        String jsonStr = AlgorithmFactory.getPARSER().parse("Longinus.new",gson.toJson(newProtocol));
        NewResultProtocol newResultProtocol = gson.fromJson(jsonStr, NewResultProtocol.class);
        if(ApplicationConstants.OK_STATIC.equals(newResultProtocol.getStatus())){
            return newResultProtocol.getInstanceGuid();
        }
        log.error("longinus.new return error: {}",newResultProtocol==null? null:newResultProtocol.getStatus());
        return null;
    }
}
