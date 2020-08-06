package com.glasssix.protocol.irisviel;

public class PersonDBSearchProtocol extends PersonDBProtocolCommon {


    private double[] feature;  //人脸特征向量 128维特征向量或512维特征向量
    private int top;            //需搜索的最相似人员个数

    public double[] getFeature() {
        return feature;
    }

    public void setFeature(double[] feature) {
        this.feature = feature;
    }

    public int getTop() {
        return top;
    }

    public void setTop(int top) {
        this.top = top;
    }

}
