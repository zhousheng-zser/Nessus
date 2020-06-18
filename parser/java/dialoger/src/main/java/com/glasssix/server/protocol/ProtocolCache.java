package com.glasssix.server.protocol;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.HashMap;
import java.util.List;
import java.util.Map;


@Component
public class ProtocolCache {
    private final static Map<String,String> startedMap = new HashMap<>();


    @Autowired
    public ProtocolCache(ProtocolConfig protocolConfig) {
        List<ProtocolRegisterEntry> registers = protocolConfig.getRegisters();
        for(ProtocolRegisterEntry r: registers){
            if(r.isStart()){
                startedMap.put(r.getName(),r.getEntry());
            }
        }
    }

    public static String getEntryName(String algorithmName){
        return startedMap.get(algorithmName);
    }
}
