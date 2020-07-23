package com.glasssix.server.pipeline;

public enum EndPointEnum {

    DETECT("detect"),ALIGN("align"),FORWARD("forward"),NEW("new"),
    ADD("add"), UPDATE("update"),SEARCH("search"),DELETE("delete"),
    ADDS("adds"),UPDATES("updates"), REMOVE("remove"),REMOVES("removes"),
    Load("load");
    private String endPointName;

    EndPointEnum(String endPointName){
        this.endPointName = endPointName;
    }

    public String getEndPointName() {
        return endPointName;
    }
}
