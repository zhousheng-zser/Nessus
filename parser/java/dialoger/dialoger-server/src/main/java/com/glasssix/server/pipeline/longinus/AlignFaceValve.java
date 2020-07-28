package com.glasssix.server.pipeline.longinus;

import com.glasssix.algorithm.AlgorithmFactory;
import com.glasssix.common.util.ApplicationConstants;
import com.glasssix.protocol.NewProtocol;
import com.glasssix.protocol.result.NewResultProtocol;
import com.glasssix.server.pipeline.ThreadLocalResource;
import com.glasssix.server.pipeline.ValveHandlerCommon;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import lombok.extern.slf4j.Slf4j;
import org.apache.commons.codec.binary.Base64;
import org.opencv.core.Mat;
import org.opencv.imgproc.Imgproc;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.context.annotation.Scope;
import org.springframework.stereotype.Component;
import org.springframework.util.CollectionUtils;

import java.util.List;
import java.util.concurrent.locks.ReentrantLock;

import static org.opencv.core.CvType.CV_8UC3;

@Slf4j
@Component
@Scope("prototype")
public class AlignFaceValve extends ValveHandlerCommon {


    @Override
    public int estimate() {
        return 0;
    }

    @Autowired
    private AlgorithmFactory algorithmFactory;

    @Override
    public String getInstanceGuid() {
        String instanceGuid = null;
        String guuidKey = AlgorithmFactory.getGuuidKey(receivedRoutingKey, device);
        synchronized (algorithmFactory) {
            List<String> consumerGuuidList = algorithmFactory.getConsumerGuuidList(guuidKey);
            if (CollectionUtils.isEmpty(consumerGuuidList)) {
                instanceGuid = createInstance();
                algorithmFactory.setConsumerGuuid(guuidKey, instanceGuid);
            }else {
                instanceGuid = consumerGuuidList.remove(0);
                consumerGuuidList.add(instanceGuid);
            }
        }
        return instanceGuid;
    }

    @Override
    public String createInstance() {
        NewProtocol newProtocol = new NewProtocol();
        newProtocol.setDevice(-1);
        String jsonStr = AlgorithmFactory.getPARSER().parse("Longinus.new", gson.toJson(newProtocol));
        NewResultProtocol newResultProtocol = gson.fromJson(jsonStr, NewResultProtocol.class);
        if (ApplicationConstants.OK_STATIC.equals(newResultProtocol.getStatus())) {
            return newResultProtocol.getInstanceGuid();
        }
        log.error("longinus.new return error: {}", newResultProtocol == null ? null : newResultProtocol.getStatus());
        return null;
    }

    @Override
    public void addPropertyAsInput(JsonObject jsonObject, JsonObject oldJsonObject) {
        if (oldJsonObject == null) {
            return;
        }
        super.addPropertyAsInput(jsonObject, oldJsonObject);
        JsonElement images = oldJsonObject.get("image");
        byte[] grayImageByte = getGrayImageFromBase64(images.getAsString(), oldJsonObject.get("height").getAsInt(), oldJsonObject.get("width").getAsInt());
        byte[] base64FromImage = getBase64FromImage(grayImageByte);
        jsonObject.addProperty("gray", new String(base64FromImage));

    }

    public byte[] getBase64FromImage(byte[] imageByte) {
        String encode = Base64.encodeBase64String(imageByte);
        return encode.getBytes();
    }

    public byte[] getGrayImageFromBase64(String imageBase64Str, int height, int width) {
        byte[] bytes = Base64.decodeBase64(imageBase64Str);
        Mat mat = new Mat(height, width, CV_8UC3);
        mat.put(0, 0, bytes);
        Mat dstImage = new Mat();
        Imgproc.cvtColor(mat, dstImage, Imgproc.COLOR_BGR2GRAY, 0);
        //Imgcodecs.imwrite("gray1-mat.jpg",dstImage);
        byte[] grayData = new byte[dstImage.cols() * dstImage.rows()];
        dstImage.get(0, 0, grayData);
        return grayData;
    }

}
