package com.glasssix.server.protocol.irisviel;

import com.glasssix.server.rabbitmq.CustomerRabbitMQSender;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

@Component
public class PersonDBLoadProtocolTest {

    @Autowired
    private CustomerRabbitMQSender customerRabbitMQSender;

    public String getPersonDBLoadProtocolData(){
        String data = "{\"event_id\":\"123456\",\"instance_id\":\"456667\",\"reserved\":\"\"}";
        return data;
    }

    public void sendPersonDBLoadProtocolData(String customerRoutingKeyPrefix){
        customerRabbitMQSender.customerSend(customerRoutingKeyPrefix+"irisviel.personDBLoad", getPersonDBLoadProtocolData());
    }
}
