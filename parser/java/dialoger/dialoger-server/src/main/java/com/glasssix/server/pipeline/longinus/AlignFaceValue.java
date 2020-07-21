package com.glasssix.server.pipeline.longinus;

import com.glasssix.algorithm.AlgorithmFactory;
import com.glasssix.common.util.ApplicationConstants;
import com.glasssix.protocol.NewProtocol;
import com.glasssix.protocol.result.NewResultProtocol;
import com.glasssix.server.pipeline.ThreadLocalResource;
import com.glasssix.server.pipeline.ValveHandlerCommon;
import com.google.gson.Gson;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import lombok.extern.slf4j.Slf4j;
import org.apache.commons.codec.binary.Base64;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;
import sun.misc.BASE64Decoder;

import javax.imageio.ImageIO;
import java.awt.image.BufferedImage;
import java.io.ByteArrayInputStream;
import java.io.IOException;

@Slf4j
@Component
public class AlignFaceValue extends ValveHandlerCommon {



    @Override
    public int estimate() {
        return 0;
    }

    @Override
    public String getInstanceGuid() {
        String instanceGuid = ThreadLocalResource.longinusInstance.get();
        if(instanceGuid == null){
            instanceGuid = createInstance();
        }
        return instanceGuid;
    }

    @Override
    public String createInstance() {
        NewProtocol newProtocol = new NewProtocol();
        newProtocol.setDevice(-1);
        String jsonStr = AlgorithmFactory.getPARSER().parse("longinus.new",gson.toJson(newProtocol));
        NewResultProtocol newResultProtocol = gson.fromJson(jsonStr, NewResultProtocol.class);
        if(ApplicationConstants.OK_STATIC.equals(newResultProtocol.getStatus())){
            ThreadLocalResource.longinusInstance.set(newResultProtocol.getInstanceGuid());
            return newResultProtocol.getInstanceGuid();
        }
        log.error("longinus.new return error: {}",newResultProtocol==null? null:newResultProtocol.getStatus());
        return null;
    }

    @Override
    public void addPropertyAsInput(JsonObject jsonObject, JsonObject oldJsonObject){
        if(oldJsonObject == null){
            return;
        }
        super.addPropertyAsInput(jsonObject,oldJsonObject);
        JsonElement images = oldJsonObject.get("image");
        byte[] grayFromImageByte = getGrayFromImageByte(Base64.decodeBase64(images.getAsString()), oldJsonObject.get("height").getAsInt(), oldJsonObject.get("width").getAsInt());
        //byte[] grayImage = getGrayFromImage(getImageFromBase64(images.getAsString()));
        byte[] base64FromImage = getBase64FromImage(grayFromImageByte);
        jsonObject.addProperty("gray",new String(base64FromImage));

    }

    public byte[] getBase64FromImage(byte[] imageByte){
        String encode = Base64.encodeBase64String(imageByte);
        return encode.getBytes();
    }

    public BufferedImage getImageFromBase64(String imageBase64Str){
        byte[] bytes = Base64.decodeBase64(imageBase64Str);
        ByteArrayInputStream byteArrayInputStream = new ByteArrayInputStream(bytes);
        BufferedImage image = null;
        try {
            image = ImageIO.read(byteArrayInputStream);
        } catch (IOException e) {
            e.printStackTrace();
        }
        return image;
        /*BASE64Decoder decoder = new sun.misc.BASE64Decoder();
        BufferedImage bi1 = null;
        try {
            byte[] bytes1 = decoder.decodeBuffer(imageBase64Str);
            for (int i = 0; i < bytes1.length; ++i) {
                if (bytes1[i] < 0) {// 调整异常数据
                    bytes1[i] += 256;
                }
            }
            ByteArrayInputStream bais = new ByteArrayInputStream(bytes1);
            bi1= ImageIO.read(bais);
        } catch (IOException e) {
            e.printStackTrace();
        }
        return bi1;*/
    }

    public byte[] getGrayFromImage(BufferedImage image){
        int width = image.getWidth();
        int height = image.getHeight();
        byte[] bytes = new byte[width * height];
        for (int j = 0; j < height; j++) {
            for (int i = 0; i < width; i++) {
                int p = image.getRGB(i, j);
                int a = (p >> 24) & 0xff;
                int r = (p >> 16) & 0xff;
                int g = (p >> 8) & 0xff;
                int b = p & 0xff;
                int avg = (r + g + b) / 3;
                p = (a << 24) | (avg << 16) | (avg << 8) | avg;
                bytes[width*(j)+i]= (byte) p;
            }
        }
        return bytes;
    }

    public byte[] getGrayFromImageByte(byte[] images,int height,int width){
        for (int j = 0; j < height; j++) {
            for (int i = 0; i < width; i++) {
                int baseNum = j*height+i*width+i;
                byte b1 = images[baseNum];
                byte g1 = images[baseNum+1];
                byte r1 = images[baseNum+2];
                byte avg1 = (byte) ((b1+g1+r1)/3);
                images[baseNum] = avg1;
                images[baseNum+1] = avg1;
                images[baseNum+2] = avg1;
            }
        }
        return images;
    }
}
