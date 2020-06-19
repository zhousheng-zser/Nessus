package com.glasssix.server.protocol.irisviel;

import com.glasssix.server.protocol.ProtocolInterface;
import com.google.gson.annotations.SerializedName;

public class PersonDBInitProtocol implements ProtocolInterface {
    @SerializedName("even_id")
    private String eventId;	    //  事件id

    @SerializedName("instance_id")
    private String instanceId;  //人员库实例id

    @SerializedName("single_database_capacity")
    private int singleDatabaseCapacity;     //单个内存映射文件保存的人脸信息条数上限
    private int dimension;      //特征向量长度

    @SerializedName("working_directory")
    private String workingDirectory;//构建映射文件和索引文件的文件夹路径

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

    public int getSingleDatabaseCapacity() {
        return singleDatabaseCapacity;
    }

    public void setSingleDatabaseCapacity(int singleDatabaseCapacity) {
        this.singleDatabaseCapacity = singleDatabaseCapacity;
    }

    public int getDimension() {
        return dimension;
    }

    public void setDimension(int dimension) {
        this.dimension = dimension;
    }

    public String getWorkingDirectory() {
        return workingDirectory;
    }

    public void setWorkingDirectory(String workingDirectory) {
        this.workingDirectory = workingDirectory;
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
