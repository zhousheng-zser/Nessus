package com.glasssix.server.pipeline;

public enum EndPointEnum {

    DETECT("detect"),ALIGN("align"),FORWARD("forward"),NEW("new"),
    ADD("add"), UPDATE("update"),SEARCH("search"),DELETE("delete"),
    ADDS("adds"),UPDATES("updates"), REMOVE("remove"),REMOVES("removes"),
    LOAD("load"),REMOVE_ALL("remove_all");
    private String endPointName;

    EndPointEnum(String endPointName){
        this.endPointName = endPointName;
    }

    public String getEndPointName() {
        return endPointName;
    }
}
