package com.glasssix.server.protocol;

public interface ProtocolInterface {


    /**
     * 每个协议自己需要处理的事情
     * @param receivedRoutingKey
     * @param correlationDate
     * @return
     */
    String protocolProcess(String receivedRoutingKey,String correlationDate);


}
