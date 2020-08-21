package com.glasssix.server.protocol.irisviel;

import com.glasssix.protocol.irisviel.PersonDBAddRecordProtocol;
import com.glasssix.protocol.irisviel.PersonDBRecord;
import com.glasssix.server.rabbitmq.CustomerRabbitMQSender;
import com.google.gson.Gson;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

@Component
public class PersonDBAddRecordProtocolTest {

    @Autowired
    private CustomerRabbitMQSender customerRabbitMQSender;
    @Autowired
    private Gson gson;

    public String getPersonDBAddRecordProtocolData(){
        String data = "{\"event_id\":\"123456\",\"instance_id\":\"456667\",\"data\":{\"feature\":[],\"key\":\"\"},\"reserved\":\"\"}";
        PersonDBAddRecordProtocol personDBAddRecordProtocol = gson.fromJson(data, PersonDBAddRecordProtocol.class);
        PersonDBRecord personDBRecord = personDBAddRecordProtocol.getData();
        double[] feature = new double[512];
        for(int i=0;i<feature.length;i++){
            feature[i] = Math.random();
        }
        personDBRecord.setFeature(feature);
        return gson.toJson(personDBAddRecordProtocol);
    }

    public void sendPersonDBAddRecordProtocolData(String customerRoutingKeyPrefix){
        customerRabbitMQSender.customerSend(customerRoutingKeyPrefix+"irisviel.personDBAddRecord", getPersonDBAddRecordProtocolData());
    }
}
