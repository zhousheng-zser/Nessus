package com.glasssix.server.algorithm;

import com.glasssix.parser.Parser;
import com.glasssix.server.protocol.ProtocolCommon;
import lombok.extern.slf4j.Slf4j;
import org.springframework.stereotype.Component;

import java.util.*;

@Slf4j
@Component
public class AlgorithmFactory {

    private static Parser PARSER = Parser.Instance();
    private Map<String, List<String>> GuuidMap = new HashMap<>();

    AlgorithmFactory(){
        log.info("filePath:"+System.getProperties().getProperty("user.dir"));
        String initPluginResult = PARSER.initPlugin("plugin_configure.json");
        log.info("initPluginResult:"+initPluginResult);
    }

    public static Parser getPARSER(){
        return PARSER;
    }

    public static String getGuuidKey(String receivedRoutingKey, ProtocolCommon protocolCommon){
        String[] routingItems = receivedRoutingKey.split("\\.");
        StringBuffer resultBuffer = new StringBuffer(routingItems[3])
                .append(".")
                .append(routingItems[4])
                .append(".")
                .append(protocolCommon.device == -1? "CPU":"GPU");
        return new String(resultBuffer);
    }

    public synchronized boolean setConsumerGuuid(String key,String guuid){
        boolean flag = true;
        List<String> consumerGuuidList = GuuidMap.get(key);
        if(consumerGuuidList == null){
            consumerGuuidList = Collections.synchronizedList(new ArrayList<String>());
            GuuidMap.put(key,consumerGuuidList);
        }
        if(consumerGuuidExist(key,guuid)){
            log.error("consumer({}) guuid({}) has exist!",key,guuid);
            flag = false;
        }else {
            consumerGuuidList.add(guuid);
        }
        return flag;
    }

    public synchronized List<String> getConsumerGuuidList(String key){
        return GuuidMap.get(key);
    }

    public synchronized boolean consumerGuuidExist(String key,String guuid){
        List<String> consumerGuuidList = GuuidMap.get(key);
        boolean flag = false;
        if(consumerGuuidList != null) {
            for (String uuid : consumerGuuidList) {
                if (uuid.equals(guuid)) {
                    flag = true;
                    break;
                }
            }
        }
        return flag;
    }

    public synchronized void deleteConsumerGuuid(String key,String guuid){
        List<String> guuidList = GuuidMap.get(key);
        guuidList.remove(guuid);
    }



}
