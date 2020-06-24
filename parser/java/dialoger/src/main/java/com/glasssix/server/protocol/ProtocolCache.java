package com.glasssix.server.protocol;

import lombok.extern.slf4j.Slf4j;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.HashMap;
import java.util.List;
import java.util.Map;


@Slf4j
@Component
public class ProtocolCache {
    private final static Map<String,String> startedMap = new HashMap<>();


    @Autowired
    public ProtocolCache(ProtocolConfig protocolConfig) {
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
