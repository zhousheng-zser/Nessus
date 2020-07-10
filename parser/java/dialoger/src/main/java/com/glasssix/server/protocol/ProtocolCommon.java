package com.glasssix.server.protocol;

import com.glasssix.server.algorithm.AlgorithmFactory;
import com.glasssix.server.util.ApplicationConstants;
import com.glasssix.server.util.SpringUtil;
import com.google.gson.annotations.Expose;
import com.google.gson.annotations.SerializedName;
import lombok.extern.slf4j.Slf4j;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.List;

@Slf4j
@Component
public abstract class ProtocolCommon {

    @SerializedName("event_id")
    public String eventId;	    //  事件id
    public int autoAssignmentInstance;  //自动分配算法实例(0：自动分配，1：手动指定)
    @SerializedName("instance_guid")
    public String instanceGuid;//算法实例id
    public int device;         //设备索引   -1:CPU, 0:GPU0
    public String reserved;    //	保留字段

    public transient AlgorithmFactory algorithmFactory;

    @Autowired
    public ProtocolCommon(){
        this.algorithmFactory = (AlgorithmFactory) SpringUtil.getApplicationContext().getBean("algorithmFactory");
    }

    public String getEventId() {
        return eventId;
    }

    public void setEventId(String eventId) {
        this.eventId = eventId;
    }

    public int getAutoAssignmentInstance() {
        return autoAssignmentInstance;
    }

    public void setAutoAssignmentInstance(int autoAssignmentInstance) {
        this.autoAssignmentInstance = autoAssignmentInstance;
    }

    public String getInstanceGuid() {
        return instanceGuid;
    }

    public void setInstanceGuid(String instanceGuid) {
        this.instanceGuid = instanceGuid;
    }

    public int getDevice() {
        return device;
    }

    public void setDevice(int device) {
        this.device = device;
    }

    public String getReserved() {
        return reserved;
    }

    public void setReserved(String reserved) {
        this.reserved = reserved;
    }

    /**
     * 每个协议自己需要处理的事情
     * @param receivedRoutingKey
     * @param correlationDate
     * @return
     */
    public String protocolProcess(String receivedRoutingKey,String correlationDate,ProtocolCommon protocolCommon){
        String guuidKey = algorithmFactory.getGuuidKey(receivedRoutingKey,protocolCommon);
        String processResult;
        switch (autoAssignmentInstance){
            case 0:
                processResult = autoAssignmentInstanceGuid(receivedRoutingKey,guuidKey,correlationDate);
                break;
            case 1:
                processResult = manualAssignmentInstanceGuid(receivedRoutingKey,guuidKey,correlationDate);
                break;
            default:
                processResult = "autoAssignmentInstance("+autoAssignmentInstance
                        +") is error value in correlationDate("+correlationDate
                        +") consumer("+receivedRoutingKey+")";
                log.error(processResult);
        }
        return processResult;
    }

    private String manualAssignmentInstanceGuid(String receivedRoutingKey,String guuidKey,String correlationDate) {
        StringBuffer resultBuffer = new StringBuffer(ApplicationConstants.OK_STATIC);
        if (!algorithmFactory.consumerGuuidExist(guuidKey, instanceGuid)) {
            resultBuffer = new StringBuffer("Manual assignment instanceGuid(")
                    .append(instanceGuid)
                    .append(") correlationDate(")
                    .append(correlationDate)
                    .append(") consumer(")
                    .append(receivedRoutingKey)
                    .append(") is not exist!");
            log.error(new String(resultBuffer));
        }
        return new String(resultBuffer);
    }

    private String autoAssignmentInstanceGuid(String receivedRoutingKey,String guuidKey,String correlationDate) {
        List<String> consumerGuuidList = algorithmFactory.getConsumerGuuidList(guuidKey);
        StringBuffer resultBuffer = new StringBuffer(ApplicationConstants.OK_STATIC);
        if(consumerGuuidList == null){
            resultBuffer = new StringBuffer("Before correlationDate(")
                    .append(correlationDate)
                    .append(") consumer(")
                    .append(receivedRoutingKey)
                    .append(") is not create instances");
            log.error(new String(resultBuffer));
        }else {
            String guuid = consumerGuuidList.remove(0);
            instanceGuid = guuid;
            consumerGuuidList.add(guuid);
        }
        return new String(resultBuffer);
    }


}
