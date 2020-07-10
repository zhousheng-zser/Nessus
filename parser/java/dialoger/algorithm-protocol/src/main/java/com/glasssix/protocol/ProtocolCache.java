package com.glasssix.protocol;

import lombok.extern.slf4j.Slf4j;

import java.util.HashMap;
import java.util.List;
import java.util.Map;


@Slf4j
public class ProtocolCache {
    private final static Map<String,String> startedMap = new HashMap<>();



    public ProtocolCache(String configFileName) {
        ProtocolConfig protocolConfig = new ProtocolConfig(configFileName);
        log.info("start load protocol register info!");
        List<ProtocolRegisterEntry> registers = protocolConfig.getRegisters();
        for(ProtocolRegisterEntry r: registers){
            if(r.isStart()){
                startedMap.put(r.getName(),r.getEntry());
            }
        }
        log.info("loaded register info all({}) and started({})!",registers.size(),startedMap.entrySet().size());
    }

    public static String getEntryName(String algorithmName){
        return startedMap.get(algorithmName);
    }
}
