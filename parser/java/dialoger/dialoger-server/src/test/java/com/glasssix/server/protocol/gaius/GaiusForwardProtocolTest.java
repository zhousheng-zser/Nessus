package com.glasssix.server.protocol.gaius;

import com.glasssix.protocol.gaius.ForwardProtocol;
import com.glasssix.server.rabbitmq.CustomerRabbitMQSender;
import com.google.gson.Gson;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

@Component
public class GaiusForwardProtocolTest {

    @Autowired
    private CustomerRabbitMQSender customerRabbitMQSender;
    @Autowired
    private Gson gson;

    public String getGaiusForwardProtocolData(String image){
        String data = "{\"event_id\":\"123456\",\"alignedImage\":\"\",\"num\":2,\"order\":0,\"reserved\":\"\"}";
        ForwardProtocol forwardProtocol = gson.fromJson(data, ForwardProtocol.class);
        forwardProtocol.setAlignedImage(image);
        return gson.toJson(forwardProtocol);
    }

    public void sendGaiusForwardProtocolData(String customerRoutingKeyPrefix,String image){
        customerRabbitMQSender.customerSend(customerRoutingKeyPrefix+"gaius.gaiusForward", getGaiusForwardProtocolData(image));
    }
}
