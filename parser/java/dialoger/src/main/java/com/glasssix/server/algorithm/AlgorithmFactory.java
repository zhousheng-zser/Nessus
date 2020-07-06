package com.glasssix.server.algorithm;

import com.glasssix.parser.Parser;
import com.glasssix.server.protocol.ProtocolCommon;
import lombok.extern.slf4j.Slf4j;
import org.springframework.stereotype.Component;

import java.util.*;
import java.util.stream.Collectors;

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
        log.debug("getGuuidKey({},{})",receivedRoutingKey,protocolCommon);
        String[] routingItems = receivedRoutingKey.split("\\.");
        StringBuffer resultBuffer = new StringBuffer(routingItems[3])
                .append(".")
                .append(routingItems[4])
                .append(".")
                .append(protocolCommon.device == -1? "CPU":"GPU");
        log.debug("getGuuidKey({},{}) return ({})",receivedRoutingKey,protocolCommon,new String(resultBuffer));
        return new String(resultBuffer);
    }

    public synchronized boolean setConsumerGuuid(String key,String guuid){
        boolean flag = true;
        log.debug("setConsumerGuuid({},{})",key,guuid);
        List<String> consumerGuuidList = GuuidMap.get(key);
        if(consumerGuuidList == null){
            log.debug("consumerGuuidList is empty with key{}",key);
            consumerGuuidList = Collections.synchronizedList(new ArrayList<String>());
            GuuidMap.put(key,consumerGuuidList);
        }
        if(consumerGuuidExist(key,guuid)){
            log.error("consumer({}) guuid({}) has exist!",key,guuid);
            flag = false;
        }else {
            consumerGuuidList.add(guuid);
            log.debug("setConsumerGuuid({},{}) success",key,guuid);
        }
        return flag;
    }

    public synchronized List<String> getConsumerGuuidList(String key){
        log.debug("getConsumerGuuidList({}",key);
        log.debug("getConsumerGuuidList({} return ({})",key,GuuidMap.get(key).stream().collect(Collectors.joining("_")));
        return GuuidMap.get(key);
    }

    public synchronized boolean consumerGuuidExist(String key,String guuid){
        log.debug("consumerGuuidExist({},{})",key,guuid);
        List<String> consumerGuuidList = GuuidMap.get(key);
        boolean flag = false;
        if(consumerGuuidList != null) {
            log.debug("consumerGuuidList is not empty with key({})",key);
            for (String uuid : consumerGuuidList) {
                if (uuid.equals(guuid)) {
                    flag = true;
                    log.debug("consumerGuuidList is exist guuid({}) with key({})",guuid,key);
                    break;
                }
            }
        }
        return flag;
    }

    public synchronized void deleteConsumerGuuid(String key,String guuid){
        log.debug("deleteConsumerGuuid({},{})",key,guuid);
        List<String> guuidList = GuuidMap.get(key);
        guuidList.remove(guuid);
        log.debug("deleteConsumerGuuid({},{}) is success",key,guuid);
    }



}
