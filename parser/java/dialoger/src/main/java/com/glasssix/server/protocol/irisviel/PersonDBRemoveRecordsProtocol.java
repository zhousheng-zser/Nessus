package com.glasssix.server.protocol.irisviel;

import com.glasssix.server.protocol.ProtocolInterface;
import com.google.gson.annotations.SerializedName;

public class PersonDBRemoveRecordsProtocol implements ProtocolInterface {

    @SerializedName("even_id")
    private String eventId;	    //  事件id

    @SerializedName("instance_id")
    private String instanceId;  //人员库实例id

    private String[] keys;      //多个人员的键值 键值长度32个字节

    private String reserved;    //	保留字段

    public String getEventId() {
        return eventId;
    }

    public void setEventId(String eventId) {
        this.eventId = eventId;
    }

    public String getInstanceId() {
        return instanceId;
    }

    public void setInstanceId(String instanceId) {
        this.instanceId = instanceId;
    }

    public String[] getKeys() {
        return keys;
    }

    public void setKeys(String[] keys) {
        this.keys = keys;
    }

    public String getReserved() {
        return reserved;
    }

    public void setReserved(String reserved) {
        this.reserved = reserved;
    }

    @Override
    public String protocolProcess(String receivedRoutingKey, String correlationDate) {
        return null;
    }
}
