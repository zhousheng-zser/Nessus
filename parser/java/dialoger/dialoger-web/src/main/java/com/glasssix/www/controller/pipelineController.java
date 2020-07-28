package com.glasssix.www.controller;

import com.glasssix.server.pipeline.PipelineContext;
import com.google.gson.Gson;
import com.google.gson.JsonObject;
import com.sun.istack.internal.NotNull;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Controller;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.ResponseBody;

import java.util.UUID;

@Controller
@RequestMapping("/pipeline")
public class pipelineController {
    @Autowired
    private PipelineContext pipelineContext;
    @Autowired
    private Gson gson;

    @RequestMapping("exec")
    @ResponseBody
    public String executor(@NotNull String receivedRoutingKey, @NotNull String message){
        JsonObject jsonObject = gson.fromJson(message,JsonObject.class);
        String correlationDate = "HTTP"+ UUID.randomUUID().toString();
        jsonObject.addProperty("event_id", "HTTP"+ UUID.randomUUID().toString());
        String execResult = pipelineContext.exec(receivedRoutingKey, correlationDate, jsonObject);
        return execResult;
    }
}
