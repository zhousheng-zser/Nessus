package com.glasssix.protocol;

import com.glasssix.algorithm.AlgorithmFactory;
import com.glasssix.common.util.ApplicationConstants;
import lombok.extern.slf4j.Slf4j;
import org.springframework.util.StringUtils;

@Slf4j
public class DeleteProtocol extends ProtocolCommon {
    @Override
    public String protocolProcess(String receivedRoutingKey,String correlationDate,ProtocolCommon protocolCommon){
        StringBuffer resultBuffer = new StringBuffer(ApplicationConstants.OK_STATIC);
        if(StringUtils.isEmpty(protocolCommon.instanceGuid)){
            resultBuffer = new StringBuffer("instance_guid is empty in correlationDate(")
                    .append(correlationDate)
                    .append(") consumer(")
                    .append(receivedRoutingKey)
                    .append("),please manual input the instance_guid which need to be remove");
            log.error(new String(resultBuffer));
        }else{
            String guuidKey = AlgorithmFactory.getGuuidKey(receivedRoutingKey,protocolCommon.getDevice());
            if(!ALGORITHM_FACTORY.consumerGuuidExist(guuidKey,protocolCommon.instanceGuid)){
                resultBuffer = new StringBuffer("Manual assignment instanceGuid(")
                        .append(instanceGuid)
                        .append(") correlationDate(")
                        .append(correlationDate)
                        .append(") consumer(")
                        .append(receivedRoutingKey)
                        .append(") is not exist!");
                log.error(new String(resultBuffer));
            }
        }

        return new String(resultBuffer);
    }

}
