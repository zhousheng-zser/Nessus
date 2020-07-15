package com.glasssix.protocol.cassius;

import com.glasssix.protocol.ProtocolCommon;
import com.google.gson.annotations.SerializedName;

public class ForwardProtocol extends ProtocolCommon {

    @SerializedName("aligned_images")
    private String[] alignedImages;//对齐生成的人脸三通道图片,每张图片为3x512x512个字节
    private int num;            //alignedImage包含的图片数
    private int order;	        //	图片的像素排列方式	0:NCHW, 1:NHWC

    public String[] getAlignedImages() {
        return alignedImages;
    }

    public void setAlignedImages(String[] alignedImages) {
        this.alignedImages = alignedImages;
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

