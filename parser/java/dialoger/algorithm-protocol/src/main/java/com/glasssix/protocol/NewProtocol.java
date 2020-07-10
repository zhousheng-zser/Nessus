package com.glasssix.protocol;

import com.glasssix.common.util.ApplicationConstants;

public class NewProtocol extends ProtocolCommon {

    @Override
    public String protocolProcess(String receivedRoutingKey,String correlationDate,ProtocolCommon protocolCommon){
        return ApplicationConstants.OK_STATIC;
    }
}
