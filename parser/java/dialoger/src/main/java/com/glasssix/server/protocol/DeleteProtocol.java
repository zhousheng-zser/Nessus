package com.glasssix.server.protocol;

import com.glasssix.server.protocol.ProtocolCommon;
import com.glasssix.server.util.ApplicationConstants;

public class DeleteProtocol extends ProtocolCommon {

    @Override
    public String protocolProcess(String receivedRoutingKey, String correlationDate, ProtocolCommon protocolCommon) {
        return ApplicationConstants.OK_STATIC;
    }
}
