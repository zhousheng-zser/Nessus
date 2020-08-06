package com.glasssix.protocol.irisviel;

public class PersonDBUpdateRecordsProtocol extends PersonDBProtocolCommon {



    private PersonDBRecord[] data; //多个人员的包含feature和key的信息

    public PersonDBRecord[] getData() {
        return data;
    }

    public void setData(PersonDBRecord[] data) {
        this.data = data;
    }

}
