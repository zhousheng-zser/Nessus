package com.glasssix.server;

import com.glasssix.server.protocol.cassius.CassiusForwardProtocolTest;
import com.glasssix.server.protocol.gaius.GaiusForwardProtocolTest;
import com.glasssix.server.protocol.irisviel.*;
import com.glasssix.server.protocol.longinus.*;
import com.glasssix.server.protocol.ProtocolConfig;
import com.glasssix.server.protocol.ProtocolRegisterEntry;
import com.glasssix.server.rabbitmq.CustomerRabbitMQSender;
import com.glasssix.server.rabbitmq.RabbitMQSender;
import com.google.gson.Gson;
import lombok.extern.slf4j.Slf4j;
import org.junit.jupiter.api.Test;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;

import java.io.*;
import java.util.List;

@SpringBootTest
@Slf4j
class DialogerApplicationTests {

	@Autowired
	private CustomerRabbitMQSender customerRabbitMQSender;

	@Autowired
	private Gson gson;

	@Autowired
	private ProtocolConfig protocolConfig;

	@Autowired
	private DetectRetinaProtocolTest detectRetinaProtocolTest;

	@Autowired
	private AlignFaceProtocolTest alignFaceProtocolTest;

	@Autowired
	private PersonDBInitProtocolTest personDBInitProtocolTest;

	@Autowired
	private PersonDBDeleteProtocolTest dbDeleteProtocolTest;

	@Autowired
	private PersonDBQueryProtocolTest dbQueryProtocolTest;

	@Autowired
	private PersonDBSearchProtocolTest dbSearchProtocolTest;

	@Autowired
	private PersonDBClearProtocolTest dbClearProtocolTest;

	@Autowired
	private PersonDBRemoveAllProtocolTest dbRemoveAllProtocolTest;

	@Autowired
	private PersonDBLoadProtocolTest loadProtocolTest;

	@Autowired
	private PersonDBRemoveRecordsProtocolTest removeRecordsProtocolTest;

	@Autowired
	private PersonDBRemoveRecordProtocolTest removeRecordProtocolTest;

	@Autowired
	private PersonDBAddRecordsProtocolTest addRecordsProtocolTest;

	@Autowired
	private PersonDBAddRecordProtocolTest addRecordProtocolTest;

	@Autowired
	private PersonDBUpdateRecordProtocolTest updateRecordProtocolTest;

	@Autowired
	private PersonDBUpdateRecordsProtocolTest updateRecordsProtocolTest;

	@Autowired
	private GaiusForwardProtocolTest gaiusForwardProtocolTest;

	@Autowired
	private CassiusForwardProtocolTest cassiusForwardProtocolTest;

	@Autowired
	private NewProtocolTest newProtocolTest;

	private String customerRoutingKey = "Glasssix.Excalibur.V1.111.";



	@Test
	void rabbitMQConnectTest() {
		log.info("start test!");
		String message = null;
		try{
			FileInputStream fileInputStream = new FileInputStream("es.txt");
			InputStreamReader inputStreamReader = new InputStreamReader(fileInputStream);
			BufferedReader bufferedReader = new BufferedReader(inputStreamReader);
			message = bufferedReader.readLine();
		} catch (FileNotFoundException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}
		while (true) {
			customerRabbitMQSender.customerSend(customerRoutingKey+"Longinus.detectEx",message);
		}
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

	/*@Test
	void threadPoolTest() throws InterruptedException {
		for(int i=0;i<100;i++){
			customerRabbitMQSender.customerSend(customerRoutingKey,""+i+"-->"+getJSON());
		}
		Thread.currentThread().join();
	}*/

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

	public String getImage(){
		String image = null;
		try{
			FileInputStream fileInputStream = new FileInputStream("es.txt");
			InputStreamReader inputStreamReader = new InputStreamReader(fileInputStream);
			BufferedReader bufferedReader = new BufferedReader(inputStreamReader);
			String message = bufferedReader.readLine();
			DetectExProtocol detectExProtocol = gson.fromJson(message, DetectExProtocol.class);
			image = detectExProtocol.getImage();
		} catch (FileNotFoundException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}
		return image;
	}

	public void waitMe(){
		try {
			Thread.sleep(1000);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
	}
	//--------------------------------------协议单元测试------------------------------------------------------------------
	@Test
	void detectRetinaTest(){
		detectRetinaProtocolTest.sendDetectRetinaProtocolData(customerRoutingKey,getImage());
		waitMe();
	}

	@Test
	void alignFaceTest(){
		alignFaceProtocolTest.sendAlignFaceProtocolData(customerRoutingKey);
		waitMe();
	}

	@Test
	void personDBInit(){
		personDBInitProtocolTest.sendPersonDBInitProtocolData(customerRoutingKey);
		waitMe();
	}

	@Test
	void personDBDelete(){
		dbDeleteProtocolTest.sendPersonDBDeleteProtocolData(customerRoutingKey);
		waitMe();
	}

	@Test
	void personDBQuery(){
		dbQueryProtocolTest.sendPersonDBQueryProtocolData(customerRoutingKey);
		waitMe();
	}

	@Test
	void personDBSearch(){
		dbSearchProtocolTest.sendPersonDBSearchProtocolData(customerRoutingKey);
		waitMe();
	}

	@Test
	void personDBClear(){
		dbClearProtocolTest.sendPersonDBClearProtocolData(customerRoutingKey);
		waitMe();
	}

	@Test
	void personDBRemoveAll(){
		dbRemoveAllProtocolTest.sendPersonDBRemoveAllProtocolData(customerRoutingKey);
		waitMe();
	}

	@Test
	void personLoad(){
		loadProtocolTest.sendPersonDBLoadProtocolData(customerRoutingKey);
		waitMe();
	}

	@Test
	void personRemoveRecords(){
		removeRecordsProtocolTest.sendPersonDBRemoveRecordsProtocolData(customerRoutingKey);
		waitMe();
	}

	@Test
	void personRemoveRecord(){
		removeRecordProtocolTest.sendPersonDBRemoveRecordProtocolData(customerRoutingKey);
		waitMe();
	}

	@Test
	void personDBAddRecords(){
		addRecordsProtocolTest.sendPersonDBAddRecordsProtocolData(customerRoutingKey);
		waitMe();
	}

	@Test
	void personAddRecord(){
		addRecordProtocolTest.sendPersonDBAddRecordProtocolData(customerRoutingKey);
		waitMe();
	}

	@Test
	void personUpdateRecord(){
		updateRecordProtocolTest.sendPersonDBUpdateRecordProtocolData(customerRoutingKey);
		waitMe();
	}

	@Test
	void personDBUpdateRecords(){
		updateRecordsProtocolTest.sendPersonDBUpdateRecordsProtocolData(customerRoutingKey);
		waitMe();
	}

	@Test
	void gaiusForward(){
		gaiusForwardProtocolTest.sendGaiusForwardProtocolData(customerRoutingKey,getImage());
		waitMe();
	}

	@Test
	void cassiusForward(){
		cassiusForwardProtocolTest.sendCassiusForwardProtocolData(customerRoutingKey,getImage());
		waitMe();
	}

	@Test
	void longinusNewTest(){
		log.debug("-------------------------------------------------");
		newProtocolTest.sendNewProtocolData(customerRoutingKey);
		waitMe();
	}

}
