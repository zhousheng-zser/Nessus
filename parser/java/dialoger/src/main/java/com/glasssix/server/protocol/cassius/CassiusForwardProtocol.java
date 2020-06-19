package com.glasssix.server.protocol.cassius;

import com.glasssix.server.protocol.ProtocolInterface;
import com.google.gson.annotations.SerializedName;

public class CassiusForwardProtocol implements ProtocolInterface {

    @SerializedName("even_id")
    private String eventId;	    //  事件id

    private String alignedImage;//对齐生成的人脸三通道图片,每张图片为3x512x512个字节
    private int num;            //alignedImage包含的图片数
    private int order;	        //	图片的像素排列方式	0:NCHW, 1:NHWC
    private String reserved;    //	保留字段

    public String getEventId() {
        return eventId;
    }

    public void setEventId(String eventId) {
        this.eventId = eventId;
    }

    public String getAlignedImage() {
        return alignedImage;
    }

    public void setAlignedImage(String alignedImage) {
        this.alignedImage = alignedImage;
    }

    public int getNum() {
        return num;
    }

    public void setNum(int num) {
        this.num = num;
    }

    public int getOrder() {
        return order;
    }

    public void setOrder(int order) {
        this.order = order;
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

