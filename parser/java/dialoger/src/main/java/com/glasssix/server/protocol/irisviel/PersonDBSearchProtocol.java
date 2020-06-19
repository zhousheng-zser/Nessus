package com.glasssix.server.protocol.irisviel;

import com.glasssix.server.protocol.ProtocolInterface;
import com.google.gson.annotations.SerializedName;

public class PersonDBSearchProtocol implements ProtocolInterface {
    @SerializedName("even_id")
    private String eventId;	    //  事件id

    @SerializedName("instance_id")
    private String instanceId;  //人员库实例id

    private double[] feature;  //人脸特征向量 128维特征向量或512维特征向量
    private int top;            //需搜索的最相似人员个数
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

    public double[] getFeature() {
        return feature;
    }

    public void setFeature(double[] feature) {
        this.feature = feature;
    }

    public int getTop() {
        return top;
    }

    public void setTop(int top) {
        this.top = top;
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
