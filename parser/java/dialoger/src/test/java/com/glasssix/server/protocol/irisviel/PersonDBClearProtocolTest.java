package com.glasssix.server.protocol.irisviel;

import com.glasssix.server.rabbitmq.CustomerRabbitMQSender;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

@Component
public class PersonDBClearProtocolTest {
    @Autowired
    private CustomerRabbitMQSender customerRabbitMQSender;

    public String getPersonDBClearProtocolData(){
        String data = "{\"event_id\":\"123456\",\"instance_id\":\"456667\",\"reserved\":\"\"}";
        return data;
    }

    public void sendPersonDBClearProtocolData(String customerRoutingKeyPrefix){
        customerRabbitMQSender.customerSend(customerRoutingKeyPrefix+"irisviel.personDBClear", getPersonDBClearProtocolData());
    }
}
