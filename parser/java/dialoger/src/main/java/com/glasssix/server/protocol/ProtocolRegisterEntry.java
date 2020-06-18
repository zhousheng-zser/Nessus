package com.glasssix.server.protocol;


import org.springframework.stereotype.Component;

@Component
public class ProtocolRegisterEntry {

    private String name;
    private boolean start;
    private String entry;

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public boolean isStart() {
        return start;
    }

    public void setStart(boolean start) {
        this.start = start;
    }

    public String getEntry() {
        return entry;
    }

    public void setEntry(String entry) {
        this.entry = entry;
    }
}
