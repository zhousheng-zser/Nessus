package com.glasssix.server.protocol.irisviel;

import com.glasssix.protocol.irisviel.PersonDBSearchProtocol;
import com.glasssix.server.rabbitmq.CustomerRabbitMQSender;
import com.google.gson.Gson;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

@Component
public class PersonDBSearchProtocolTest {
    @Autowired
    private CustomerRabbitMQSender customerRabbitMQSender;

    @Autowired
    private Gson gson;

    public String getPersonDBSearchProtocolData(){
        String data = "{\"event_id\":\"123456\",\"instance_id\":\"456667\",\"feature\":[0.1,0.1],\"top\":3,\"reserved\":\"\"}";
        PersonDBSearchProtocol personDBSearchProtocol = gson.fromJson(data, PersonDBSearchProtocol.class);
        double[] features = new double[512];
        for(int i=0;i<features.length;i++){
            features[i] = Math.random();
        }
        personDBSearchProtocol.setFeature(features);
        return gson.toJson(personDBSearchProtocol);
    }

    public void sendPersonDBSearchProtocolData(String customerRoutingKeyPrefix){
        customerRabbitMQSender.customerSend(customerRoutingKeyPrefix+"irisviel.personDBSearch", getPersonDBSearchProtocolData());
    }
}
