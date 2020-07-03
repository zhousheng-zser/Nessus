package com.glasssix.server.protocol.irisviel;

import com.glasssix.server.protocol.ProtocolCommon;
import com.google.gson.annotations.SerializedName;

public class PersonDBRemoveRecordProtocol extends PersonDBProtocolCommon {


    private String key;      //人员的键值 键值长度32个字节


    public String getKey() {
        return key;
    }

    public void setKey(String key) {
        this.key = key;
    }

}
