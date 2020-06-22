package com.glasssix.server.protocol.irisviel;

import com.glasssix.server.rabbitmq.CustomerRabbitMQSender;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

@Component
public class PersonDBInitProtocolTest {

    @Autowired
    private CustomerRabbitMQSender customerRabbitMQSender;

    public String getPersonDBInitProtocolData(){
        String data = "{\"event_id\":\"123456\",\"instance_id\":\"342454\",\"single_database_capacity\":1000," +
                "\"dimension\":128,\"working_directory\":\"\",\"reserved\":\"\"}";
        return data;
    }

    public void sendPersonDBInitProtocolData(String customerRoutingKeyPrefix){
        customerRabbitMQSender.customerSend(customerRoutingKeyPrefix+"irisviel.personDBInit", getPersonDBInitProtocolData());
    }
}
