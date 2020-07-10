package com.glasssix.protocol.irisviel;

public class PersonDBUpdateRecordProtocol extends PersonDBProtocolCommon {


    private PersonDBRecord data; //人员的包含feature和key的信息

    public PersonDBRecord getData() {
        return data;
    }

    public void setData(PersonDBRecord data) {
        this.data = data;
    }


}
