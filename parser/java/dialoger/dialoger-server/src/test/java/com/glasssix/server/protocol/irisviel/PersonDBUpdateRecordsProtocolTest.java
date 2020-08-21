package com.glasssix.server.protocol.irisviel;

import com.glasssix.protocol.irisviel.PersonDBRecord;
import com.glasssix.protocol.irisviel.PersonDBUpdateRecordsProtocol;
import com.glasssix.server.rabbitmq.CustomerRabbitMQSender;
import com.google.gson.Gson;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

@Component
public class PersonDBUpdateRecordsProtocolTest {

    @Autowired
    private CustomerRabbitMQSender customerRabbitMQSender;
    @Autowired
    private Gson gson;

    public String getPersonDBUpdateRecordsProtocolData(){
        String data = "{\"event_id\":\"123456\",\"instance_id\":\"456667\",\"data\":[{\"feature\":[],\"key\":\"\"}," +
                "{\"feature\":[],\"key\":\"\"}],\"reserved\":\"\"}";
        PersonDBUpdateRecordsProtocol personDBUpdateRecordsProtocol = gson.fromJson(data, PersonDBUpdateRecordsProtocol.class);
        PersonDBRecord[] personDBRecords = personDBUpdateRecordsProtocol.getData();
        for(PersonDBRecord p: personDBRecords){
            double[] feature = new double[512];
            for(int i=0;i<feature.length;i++){
                feature[i] = Math.random();
            }
            p.setFeature(feature);
        }
        return gson.toJson(personDBUpdateRecordsProtocol);
    }

    public void sendPersonDBUpdateRecordsProtocolData(String customerRoutingKeyPrefix){
        customerRabbitMQSender.customerSend(customerRoutingKeyPrefix+"irisviel.personDBUpdateRecords", getPersonDBUpdateRecordsProtocolData());
    }
}
