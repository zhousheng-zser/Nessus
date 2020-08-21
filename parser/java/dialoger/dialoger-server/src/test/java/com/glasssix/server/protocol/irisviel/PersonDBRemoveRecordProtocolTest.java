package com.glasssix.server.protocol.irisviel;

import com.glasssix.server.rabbitmq.CustomerRabbitMQSender;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

@Component
public class PersonDBRemoveRecordProtocolTest {

    @Autowired
    private CustomerRabbitMQSender customerRabbitMQSender;

    public String getPersonDBRemoveRecordProtocolData(){
        String data = "{\"event_id\":\"123456\",\"instance_id\":\"456667\",\"key\":\"12453\",\"reserved\":\"\"}";
        return data;
    }

    public void sendPersonDBRemoveRecordProtocolData(String customerRoutingKeyPrefix){
        customerRabbitMQSender.customerSend(customerRoutingKeyPrefix+"irisviel.personDBRemoveRecord", getPersonDBRemoveRecordProtocolData());
    }
}
