package com.glasssix.server.pipeline.irisviel;

import com.glasssix.algorithm.AlgorithmFactory;
import com.glasssix.common.util.ApplicationConstants;
import com.glasssix.protocol.result.NewResultProtocol;
import com.glasssix.server.instancemap.InstanceFileOption;
import com.glasssix.server.instancemap.InstanceMapFileOption;
import com.glasssix.server.pipeline.ValveHandlerCommon;
import com.google.gson.JsonObject;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

@Component
public class PersonDBValveCommon extends ValveHandlerCommon {

    Logger log = LoggerFactory.getLogger(PersonDBValveCommon.class);
    private String irisvielInstanceGuid;
    private String uuid;
    @Autowired
    protected AlgorithmFactory algorithmFactory;
    @Autowired
    protected InstanceMapFileOption instanceMapFileOption;
    @Autowired
    protected InstanceFileOption instanceFileOption;

    @Override
    public String handler(String receivedRoutingKey, String correlationDate, JsonObject jsonObject) {
        log.info("CALL JNI with InstanceTopic:{} and param{}",super.getInstanceTopic(), gson.toJson(jsonObject));
        if(irisvielInstanceGuid != null){
            if(algorithmFactory.consumerGuuidExist(getGuuidKey(receivedRoutingKey,-1),irisvielInstanceGuid)) {
                jsonObject.addProperty("instance_guid", irisvielInstanceGuid);
            }else{
                String instanceGuid = recreateIrisviel();
                jsonObject.addProperty("instance_guid",instanceGuid);
            }
        }
        String result = AlgorithmFactory.getPARSER().parse(super.getInstanceTopic(), gson.toJson(jsonObject));
        log.info("JNI Receive: {}",result);
        return result;
    }

    private String recreateIrisviel() {
        String oldInstanceId = instanceMapFileOption.getInstanceId(uuid);
        //String path = irisvielMapFileOption.getPath(oldInstanceId);
        String context = instanceFileOption.getContext(uuid);
        log.info("recreate Irisviel instance with  param:{}",context);
        String result = AlgorithmFactory.getPARSER().parse("Irisviel.new", context);
        log.info("recreate Irisviel instance result: {}",result);
        NewResultProtocol newResultProtocol = gson.fromJson(result, NewResultProtocol.class);
        String instanceGuid = null;
        if(ApplicationConstants.OK_STATIC.equals(newResultProtocol.getStatus())){
            instanceGuid = newResultProtocol.getInstanceGuid();
            instanceMapFileOption.update(uuid,instanceGuid);
            algorithmFactory.setConsumerGuuid(getGuuidKey(receivedRoutingKey,-1),instanceGuid);
            JsonObject jsonObject = new JsonObject();
            jsonObject.addProperty("instance_guid",instanceGuid);
            AlgorithmFactory.getPARSER().parse("Irisviel.load_databases", gson.toJson(jsonObject));
            log.info("load_databases {}",instanceGuid);
        }
        return instanceGuid;

    }

    @Override
    public int estimate() {
        return 0;
    }

    @Override
    public String getInstanceGuid() {
        return null;
    }

    @Override
    public String createInstance() {
        return null;
    }

    protected String getGuuidKey(String receivedRoutingKey, int device) {
        String guuidKey = AlgorithmFactory.getGuuidKey(receivedRoutingKey, device);
        return guuidKey+".irisviel";
    }

    public String getIrisvielInstanceGuid() {
        return irisvielInstanceGuid;
    }

    public void setIrisvielInstanceGuid(String irisvielInstanceGuid) {
        this.irisvielInstanceGuid = irisvielInstanceGuid;
    }

    public AlgorithmFactory getAlgorithmFactory() {
        return algorithmFactory;
    }

    public void setAlgorithmFactory(AlgorithmFactory algorithmFactory) {
        this.algorithmFactory = algorithmFactory;
    }

    public String getUuid() {
        return uuid;
    }

    public void setUuid(String uuid) {
        this.uuid = uuid;
    }
}
