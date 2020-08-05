package com.glasssix.server.protocol.irisviel;

import com.glasssix.protocol.irisviel.PersonDBRecord;
import com.glasssix.protocol.irisviel.PersonDBUpdateRecordProtocol;
import com.glasssix.server.rabbitmq.CustomerRabbitMQSender;
import com.google.gson.Gson;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

@Component
public class PersonDBUpdateRecordProtocolTest {

    @Autowired
    private CustomerRabbitMQSender customerRabbitMQSender;
    @Autowired
    private Gson gson;

    public String getPersonDBUpdateRecordProtocolData(){
        String data = "{\"event_id\":\"123456\",\"instance_id\":\"456667\",\"data\":{\"feature\":[],\"key\":\"\"},\"reserved\":\"\"}";
        PersonDBUpdateRecordProtocol personDBUpdateRecordProtocol = gson.fromJson(data, PersonDBUpdateRecordProtocol.class);
        PersonDBRecord personDBRecord = personDBUpdateRecordProtocol.getData();
        double[] feature = new double[512];
        for(int i=0;i<feature.length;i++){
            feature[i] = Math.random();
        }
        personDBRecord.setFeature(feature);
        return gson.toJson(personDBUpdateRecordProtocol);
    }

    public void sendPersonDBUpdateRecordProtocolData(String customerRoutingKeyPrefix){
        customerRabbitMQSender.customerSend(customerRoutingKeyPrefix+"irisviel.personDBUpdateRecord", getPersonDBUpdateRecordProtocolData());
    }
}
