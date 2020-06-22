package com.glasssix.server.protocol.cassius;

import com.glasssix.server.protocol.gaius.GaiusForwardProtocol;
import com.glasssix.server.rabbitmq.CustomerRabbitMQSender;
import com.google.gson.Gson;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

@Component
public class CassiusForwardProtocolTest {

    @Autowired
    private CustomerRabbitMQSender customerRabbitMQSender;
    @Autowired
    private Gson gson;

    public String getCassiusForwardProtocolData(String image){
        String data = "{\"event_id\":\"123456\",\"alignedImage\":\"\",\"num\":2,\"order\":0,\"reserved\":\"\"}";
        CassiusForwardProtocol cassiusForwardProtocol = gson.fromJson(data, CassiusForwardProtocol.class);
        cassiusForwardProtocol.setAlignedImage(image);
        return gson.toJson(cassiusForwardProtocol);
    }

    public void sendCassiusForwardProtocolData(String customerRoutingKeyPrefix,String image){
        customerRabbitMQSender.customerSend(customerRoutingKeyPrefix+"cassius.cassiusForward", getCassiusForwardProtocolData(image));
    }
}
