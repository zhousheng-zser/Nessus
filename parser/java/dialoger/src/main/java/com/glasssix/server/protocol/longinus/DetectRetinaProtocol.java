package com.glasssix.server.protocol.longinus;

import com.glasssix.server.protocol.ProtocolCommon;
import com.google.gson.annotations.SerializedName;

public class DetectRetinaProtocol extends ProtocolCommon {
    private String image;   //三通道图片base64编码
    private String format;  //图片压缩格式RAW/JPEG/PNG
    private int height;     //图片高度
    private int width;      //图片宽度
    private int minWin;
    private double threshold;   //模型阈值
    private int order;      //图片的像素排列方式(0:NCHW, 1:NHWC)


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

    public int getMinWin() {
        return minWin;
    }

    public void setMinWin(int minWin) {
        this.minWin = minWin;
    }

    public double getThreshold() {
        return threshold;
    }

    public void setThreshold(double threshold) {
        this.threshold = threshold;
    }

    public int getOrder() {
        return order;
    }

    public void setOrder(int order) {
        this.order = order;
    }

}
