package com.glasssix.protocol.result;

import com.google.gson.annotations.SerializedName;

public class NewResultProtocol {
    private String status;
    @SerializedName("instance_guid")
    private String instanceGuid;


    @SerializedName("event_id")
    public String eventId;	    //  事件id

    public String getStatus() {
        return status;
    }

    public void setStatus(String status) {
        this.status = status;
    }

    public String getInstanceGuid() {
        return instanceGuid;
    }

    public void setInstanceGuid(String instanceGuid) {
        this.instanceGuid = instanceGuid;
    }

    public String getEventId() {
        return eventId;
    }

    public void setEventId(String eventId) {
        this.eventId = eventId;
    }
}
