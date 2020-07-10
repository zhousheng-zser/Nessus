package com.glasssix.protocol;

import org.springframework.boot.context.properties.ConfigurationProperties;
import org.springframework.boot.env.YamlPropertySourceLoader;
import org.springframework.core.env.PropertySource;
import org.springframework.core.io.ClassPathResource;
import org.springframework.stereotype.Component;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;

@Component
public class ProtocolConfig {

    private List<ProtocolRegisterEntry> registers = null;


    public ProtocolConfig(String configFileName) {
        registers = new ArrayList<>();
        try {
            PropertySource<Map<String,Object>> propertySource = (PropertySource<Map<String, Object>>) new YamlPropertySourceLoader()
                    .load(configFileName, new ClassPathResource(configFileName)).get(0);
            Map<String, Object> source = propertySource.getSource();
            for(int i=0;i<source.size()/3;i++){
                registers.add(new ProtocolRegisterEntry(source.get("protocol.registers["+i+"].name").toString(),
                        "true".equals(source.get("protocol.registers["+i+"].start").toString()),
                        source.get("protocol.registers["+i+"].entry").toString()));
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }


    public List<ProtocolRegisterEntry> getRegisters() {
        return registers;
    }

    public void setRegisters(List<ProtocolRegisterEntry> registers) {
        this.registers = registers;
    }
}
