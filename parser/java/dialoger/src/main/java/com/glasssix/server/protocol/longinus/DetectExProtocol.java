package com.glasssix.server.protocol.longinus;

import com.glasssix.server.protocol.ProtocolInterface;
import com.google.gson.annotations.SerializedName;

import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.Base64;
import java.util.UUID;

public class DetectExProtocol implements ProtocolInterface {

    @SerializedName("even_id")
    private String eventId;	//  事件id
    private String image;		//  三通道图片	base64编码
    private String format;	    //	图片压缩格式	RAW/JPEG/PNG
    private int height;	        //	图片高度
    private int width;	        //	图片宽度
    private double[] threshold;	//  Double数组	模型阈值
    private double factor;	    //	缩放系数
    private int order;	        //	图片的像素排列方式	0:NCHW, 1:NHWC
    private int minSize;
    private String reserved;    //	保留字段


    public String getEventId() {
        return eventId;
    }

    public void setEventId(String eventId) {
        this.eventId = eventId;
    }

    public String getImage() {
        return image;
    }

    public void setImage(String image) {
        this.image = image;
    }

    public String getFormat() {
        return format;
    }

    public void setFormat(String format) {
        this.format = format;
    }

    public int getHeight() {
        return height;
    }

    public void setHeight(int height) {
        this.height = height;
    }

    public int getWidth() {
        return width;
    }

    public void setWidth(int width) {
        this.width = width;
    }

    public double[] getThreshold() {
        return threshold;
    }

    public void setThreshold(double[] threshold) {
        this.threshold = threshold;
    }

    public double getFactor() {
        return factor;
    }

    public void setFactor(double factor) {
        this.factor = factor;
    }

    public int getOrder() {
        return order;
    }

    public void setOrder(int order) {
        this.order = order;
    }

    public int getMinSize() {
        return minSize;
    }

    public void setMinSize(int minSize) {
        this.minSize = minSize;
    }

    public String getReserved() {
        return reserved;
    }

    public void setReserved(String reserved) {
        this.reserved = reserved;
    }

    @Override
    public String protocolProcess(String receivedRoutingKey,String correlationDate) {
        /*try {
            byte[] images = Base64.getDecoder().decode(image.getBytes());
            FileOutputStream fileOutputStream = new FileOutputStream("image\\" + UUID.randomUUID().toString().split("-")[0] + ".jpg");
            fileOutputStream.write(images);
            fileOutputStream.flush();
            fileOutputStream.close();
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }*/
        return null;
    }


}
