package com.glasssix.server.protocol.irisviel;

import com.glasssix.server.protocol.ProtocolCommon;
import com.google.gson.annotations.SerializedName;

public class PersonDBRemoveRecordsProtocol extends PersonDBProtocolCommon {


    private String[] keys;      //多个人员的键值 键值长度32个字节


    public String[] getKeys() {
        return keys;
    }

    public void setKeys(String[] keys) {
        this.keys = keys;
    }

}
