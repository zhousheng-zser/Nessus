package com.glasssix.server.pipeline;

public class ValveConfigEntry {
    private String name;
    private String valveClass;
    private String instanceTopic;

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public String getValveClass() {
        return valveClass;
    }

    public void setValveClass(String valveClass) {
        this.valveClass = valveClass;
    }

    public String getInstanceTopic() {
        return instanceTopic;
    }

    public void setInstanceTopic(String instanceTopic) {
        this.instanceTopic = instanceTopic;
    }
}
