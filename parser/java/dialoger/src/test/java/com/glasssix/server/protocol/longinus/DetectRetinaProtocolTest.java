package com.glasssix.server.protocol.longinus;


import com.glasssix.server.rabbitmq.CustomerRabbitMQSender;
import com.google.gson.Gson;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.io.*;
import java.util.UUID;

@Component
public class DetectRetinaProtocolTest {

    @Autowired
    private Gson gson;

    @Autowired
    private CustomerRabbitMQSender customerRabbitMQSender;

    public String getDetectRetinaProtocolData(String image){
        String data = "{\"event_id\":\"123456\",\"image\": \"\",\"format\": \"JPEG\",\"height\": 240," +
                "\"width\": 320,	\"min_win\": 48,	\"threshold\": 0.5,	\"order\": 1,	\"reserved\": \"\"}";
        DetectRetinaProtocol detectRetinaProtocol = gson.fromJson(data, DetectRetinaProtocol.class);
        detectRetinaProtocol.setImage(image);
        detectRetinaProtocol.setEventId(UUID.randomUUID().toString());
        detectRetinaProtocol.setAutoAssignmentInstance(0);
        return gson.toJson(detectRetinaProtocol);
    }

    public void sendDetectRetinaProtocolData(String customerRoutingKeyPrefix,String image){
        customerRabbitMQSender.customerSend(customerRoutingKeyPrefix+"Longinus.detectRetina",
                getDetectRetinaProtocolData(image));
    }
}
