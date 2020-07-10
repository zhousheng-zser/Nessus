package com.glasssix.server.protocol.longinus;

import com.glasssix.server.rabbitmq.CustomerRabbitMQSender;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

@Component
public class AlignFaceProtocolTest {

    @Autowired
    private CustomerRabbitMQSender customerRabbitMQSender;

    public String getAlignFaceProtocolData(){
        String data = "{\"event_id\":\"123456\",\"gray\":\"\",\"format\":\"JPEG\",\"height\":240,\"width\":320," +
                "\"FaceRectwithFaceInfo_list\":[{\"x\":20,\"y\":20,\"width\":260,\"height\":200," +
                "\"landmark\":[{\"x\":45,\"y\":45},{\"x\":45,\"y\":45},{\"x\":45,\"y\":45}," +
                "{\"x\":45,\"y\":45},{\"x\":45,\"y\":45}]},{\"x\":20,\"y\":20,\"width\":260,\"height\":200," +
                "\"landmark\":[{\"x\":45,\"y\":45},{\"x\":45,\"y\":45},{\"x\":45,\"y\":45},{\"x\":45,\"y\":45}," +
                "{\"x\":45,\"y\":45}]}],\"reserved\":\"\"}";
        return data;
    }

    public void sendAlignFaceProtocolData(String customerRoutingKeyPrefix){
        customerRabbitMQSender.customerSend(customerRoutingKeyPrefix+"Longinus.alignFace", getAlignFaceProtocolData());
    }
}
