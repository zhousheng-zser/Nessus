package com.glasssix.server.pipeline;

import java.util.List;

public class PipelineRegistEntry {

    private String name;
    private int order;
    private List<ValveConfigEntry> valves;

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public int getOrder() {
        return order;
    }

    public void setOrder(int order) {
        this.order = order;
    }

    public List<ValveConfigEntry> getValves() {
        return valves;
    }

    public void setValves(List<ValveConfigEntry> valves) {
        this.valves = valves;
    }
}
