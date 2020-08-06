package com.glasssix.server.pipeline;

import org.springframework.boot.context.properties.ConfigurationProperties;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.List;


@Component
@ConfigurationProperties(prefix = "pipeline")
public class PipelineRegistListCache {


    private List<PipelineRegistEntry> lineNodes = new ArrayList<>();
    private List<ValveConfigEntry> signleNodes = new ArrayList<>();



    public List<PipelineRegistEntry> getLineNodes() {
        return lineNodes;
    }

    public void setLineNodes(List<PipelineRegistEntry> lineNodes) {
        this.lineNodes = lineNodes;
    }

    public List<ValveConfigEntry> getSignleNodes() {
        return signleNodes;
    }

    public void setSignleNodes(List<ValveConfigEntry> signleNodes) {
        this.signleNodes = signleNodes;
    }
}
