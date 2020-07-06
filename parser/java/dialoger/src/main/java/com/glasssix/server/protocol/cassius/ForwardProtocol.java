package com.glasssix.server.protocol.cassius;

import com.glasssix.server.protocol.ProtocolCommon;
import com.google.gson.annotations.SerializedName;

public class ForwardProtocol extends ProtocolCommon {

    private String alignedImage;//对齐生成的人脸三通道图片,每张图片为3x512x512个字节
    private int num;            //alignedImage包含的图片数
    private int order;	        //	图片的像素排列方式	0:NCHW, 1:NHWC

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
}

