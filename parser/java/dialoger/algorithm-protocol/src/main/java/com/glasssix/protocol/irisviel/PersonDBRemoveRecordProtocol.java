package com.glasssix.protocol.irisviel;

public class PersonDBRemoveRecordProtocol extends PersonDBProtocolCommon {


    private String key;      //人员的键值 键值长度32个字节


    public String getKey() {
        return key;
    }

    public void setKey(String key) {
        this.key = key;
    }

}
