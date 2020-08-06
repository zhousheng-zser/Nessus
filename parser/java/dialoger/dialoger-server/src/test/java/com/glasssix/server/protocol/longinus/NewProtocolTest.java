package com.glasssix.server.protocol.longinus;

import com.glasssix.server.rabbitmq.CustomerRabbitMQSender;
import com.google.gson.Gson;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

@Component
public class NewProtocolTest {
    @Autowired
    private Gson gson;

    @Autowired
    private CustomerRabbitMQSender customerRabbitMQSender;

    public String getNewProtocolData(){
        String data = "{\"event_id\":\"123456\",\"device\": -1,\"reserved\": \"\"}";
        return data;
    }

    public void sendNewProtocolData(String customerRoutingKeyPrefix){
        customerRabbitMQSender.customerSend(customerRoutingKeyPrefix+"Longinus.new",
                getNewProtocolData());
    }
}
