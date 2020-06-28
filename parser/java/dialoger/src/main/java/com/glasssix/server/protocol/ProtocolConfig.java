package com.glasssix.server.protocol;

import org.springframework.boot.context.properties.ConfigurationProperties;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.List;

@Component
@ConfigurationProperties(prefix = "protocol")
public class ProtocolConfig {

    private List<ProtocolRegisterEntry> registers = new ArrayList<>();

    public List<ProtocolRegisterEntry> getRegisters() {
        return registers;
    }

    public void setRegisters(List<ProtocolRegisterEntry> registers) {
        this.registers = registers;
    }
}
