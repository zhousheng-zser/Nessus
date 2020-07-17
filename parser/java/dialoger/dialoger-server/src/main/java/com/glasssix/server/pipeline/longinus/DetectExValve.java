package com.glasssix.server.pipeline.longinus;

import com.glasssix.server.pipeline.ValveHandlerCommon;

public class DetectExValve extends ValveHandlerCommon {
    @Override
    public int estimate() {
        return 0;
    }

    @Override
    public void createInstance(String instanceName) {

    }
}
