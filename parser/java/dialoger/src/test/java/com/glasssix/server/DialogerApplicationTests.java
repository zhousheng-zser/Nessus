package com.glasssix.server;

import com.glasssix.server.protocol.longinus.DetectRetinaProtocol;
import com.glasssix.server.protocol.ProtocolConfig;
import com.glasssix.server.protocol.ProtocolRegisterEntry;
import com.glasssix.server.rabbitmq.RabbitMQSender;
import com.google.gson.Gson;
import lombok.extern.slf4j.Slf4j;
import org.junit.jupiter.api.Test;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;

import java.util.List;

@SpringBootTest
@Slf4j
class DialogerApplicationTests {

	@Autowired
	private RabbitMQSender rabbitMQSender;

	@Autowired
	private Gson gson;

	@Autowired
	private ProtocolConfig protocolConfig;



	@Test
	void rabbitMQConnectTest() {
		log.info("start test!");
		rabbitMQSender.send("hello body!");
	}

	@Test
	void gsonTest(){
		String jsonStr = getJSON();
		log.info(jsonStr);
		DetectRetinaProtocol d1 = gson.fromJson(jsonStr, DetectRetinaProtocol.class);
		log.info(d1.toString());
	}

	@Test
	void log4j2Test(){
		log.trace("trace");
		log.debug("debug");
		log.warn("warn");
		log.info("info");
		log.error("error");
	}

	@Test
	void threadPoolTest() throws InterruptedException {
		for(int i=0;i<100;i++){
			rabbitMQSender.send(""+i+"-->"+getJSON());
		}
		Thread.currentThread().join();
	}

	@Test
	void protocolRegisterTest(){
		List<ProtocolRegisterEntry> registerList = protocolConfig.getRegisters();
		System.out.println(registerList.get(0).getName());
	}

	public String getJSON(){
		DetectRetinaProtocol detectRetinaProtocol = new DetectRetinaProtocol();
		detectRetinaProtocol.setEventId(""+System.currentTimeMillis());
		detectRetinaProtocol.setFormat("PNG");
		detectRetinaProtocol.setHeight(12);
		detectRetinaProtocol.setWidth(8);
		detectRetinaProtocol.setImage("ZHNkZGRqZmFzbGRmc2RhYWZkbXNrYWZqYWRpc25jJTJDYWNud2ZrcWQ=");
		String jsonStr = gson.toJson(detectRetinaProtocol);
		return jsonStr;
	}

}
