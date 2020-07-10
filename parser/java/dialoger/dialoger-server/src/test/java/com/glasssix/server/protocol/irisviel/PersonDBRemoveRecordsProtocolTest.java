package com.glasssix.server.protocol.irisviel;

import com.glasssix.server.rabbitmq.CustomerRabbitMQSender;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

@Component
public class PersonDBRemoveRecordsProtocolTest {

    @Autowired
    private CustomerRabbitMQSender customerRabbitMQSender;

    public String getPersonDBRemoveRecordsProtocolData(){
        String data = "{\"event_id\":\"123456\",\"instance_id\":\"456667\",\"keys\":[\"12453\",\"43d43\"],\"reserved\":\"\"}";
        return data;
    }

    public void sendPersonDBRemoveRecordsProtocolData(String customerRoutingKeyPrefix){
        customerRabbitMQSender.customerSend(customerRoutingKeyPrefix+"irisviel.personDBRemoveRecords", getPersonDBRemoveRecordsProtocolData());
    }
}
