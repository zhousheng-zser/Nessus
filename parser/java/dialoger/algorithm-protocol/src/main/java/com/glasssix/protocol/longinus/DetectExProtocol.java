package com.glasssix.protocol.longinus;

import com.glasssix.protocol.ProtocolCommon;

public class DetectExProtocol extends ProtocolCommon {


    private String image;		//  三通道图片	base64编码
    private String format;	    //	图片压缩格式	RAW/JPEG/PNG
    private int height;	        //	图片高度
    private int width;	        //	图片宽度
    private double[] threshold;	//  Double数组	模型阈值
    private double factor;	    //	缩放系数
    private int order;	        //	图片的像素排列方式	0:NCHW, 1:NHWC
    private int minSize;
    private int stage;


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

    public int getStage() {
        return stage;
    }

    public void setStage(int stage) {
        this.stage = stage;
    }
}
