package com.glasssix.server.pipeline;

import org.springframework.boot.context.properties.ConfigurationProperties;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.List;


@ConfigurationProperties(prefix = "pipeline")
@Component
public class PipelineRegistListCache {

    private List<PipelineRegistEntry> lineNodes = new ArrayList<>();



    public List<PipelineRegistEntry> getLineNodes() {
        return lineNodes;
    }

    public void setLineNodes(List<PipelineRegistEntry> lineNodes) {
        this.lineNodes = lineNodes;
    }
}
