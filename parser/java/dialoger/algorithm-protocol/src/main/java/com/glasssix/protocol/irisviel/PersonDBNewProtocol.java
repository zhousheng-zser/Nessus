package com.glasssix.protocol.irisviel;

import com.glasssix.protocol.ProtocolCommon;
import com.glasssix.common.util.ApplicationConstants;
import com.google.gson.annotations.SerializedName;

public class PersonDBNewProtocol extends PersonDBProtocolCommon {


    @SerializedName("single_database_capacity")
    private int singleDatabaseCapacity;     //单个内存映射文件保存的人脸信息条数上限
    private int dimension;      //特征向量长度

    @SerializedName("working_directory")
    private String workingDirectory;//构建映射文件和索引文件的文件夹路径

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

    @Override
    public String protocolProcess(String receivedRoutingKey,String correlationDate,ProtocolCommon protocolCommon){
        return ApplicationConstants.OK_STATIC;
    }
}
