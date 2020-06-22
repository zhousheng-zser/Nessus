package com.glasssix.server.protocol.irisviel;

import com.glasssix.server.rabbitmq.CustomerRabbitMQSender;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

@Component
public class PersonDBDeleteProtocolTest {

    @Autowired
    private CustomerRabbitMQSender customerRabbitMQSender;

    public String getPersonDBDeleteProtocolData(){
        String data = "{\"event_id\":\"123456\",\"instance_id\":\"342454\",\"reserved\":\"\"}";
        return data;
    }

    public void sendPersonDBDeleteProtocolData(String customerRoutingKeyPrefix){
        customerRabbitMQSender.customerSend(customerRoutingKeyPrefix+"irisviel.personDBDelete", getPersonDBDeleteProtocolData());
    }
}
