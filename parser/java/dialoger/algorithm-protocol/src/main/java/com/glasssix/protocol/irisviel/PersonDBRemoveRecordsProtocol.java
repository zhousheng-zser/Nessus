package com.glasssix.protocol.irisviel;

public class PersonDBRemoveRecordsProtocol extends PersonDBProtocolCommon {


    private String[] keys;      //多个人员的键值 键值长度32个字节


    public String[] getKeys() {
        return keys;
    }

    public void setKeys(String[] keys) {
        this.keys = keys;
    }

}
