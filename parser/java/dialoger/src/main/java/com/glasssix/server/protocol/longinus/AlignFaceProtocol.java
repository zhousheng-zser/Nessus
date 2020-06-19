package com.glasssix.server.protocol.longinus;

import com.glasssix.server.protocol.ProtocolInterface;
import com.google.gson.annotations.SerializedName;

public class AlignFaceProtocol implements ProtocolInterface {

    @SerializedName("even_id")
    private String eventId;	//  事件id
    private String gray;        //灰度图片像素
    private int channel;        //图片通道数 灰度图为1
    private int height;	        //	图片高度
    private int width;	        //	图片宽度
    @SerializedName("FaceRectwithFaceInfo_list")
    private FaceRectwithFaceInfo[] faceRectwithFaceInfos;//对象数组 人脸定位及信息
    private String reserved;    //	保留字段

    public String getEventId() {
        return eventId;
    }

    public void setEventId(String eventId) {
        this.eventId = eventId;
    }

    public String getGray() {
        return gray;
    }

    public void setGray(String gray) {
        this.gray = gray;
    }

    public int getChannel() {
        return channel;
    }

    public void setChannel(int channel) {
        this.channel = channel;
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

    public FaceRectwithFaceInfo[] getFaceRectwithFaceInfos() {
        return faceRectwithFaceInfos;
    }

    public void setFaceRectwithFaceInfos(FaceRectwithFaceInfo[] faceRectwithFaceInfos) {
        this.faceRectwithFaceInfos = faceRectwithFaceInfos;
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

    class FaceRectwithFaceInfo{
        private int x;      //人脸x坐标
        private int y;      //人脸y坐标
        private int height; //人脸高度
        private int width;  //人脸宽度
        private double confidence;  //人脸置信度
        private double yaw;         //人脸偏航角
        private double pitch;       //人脸俯仰角
        private double roll;        //人脸翻滚角
        private Landmark[] landmark;//人脸5点关键点

        public int getX() {
            return x;
        }

        public void setX(int x) {
            this.x = x;
        }

        public int getY() {
            return y;
        }

        public void setY(int y) {
            this.y = y;
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

        public double getConfidence() {
            return confidence;
        }

        public void setConfidence(double confidence) {
            this.confidence = confidence;
        }

        public double getYaw() {
            return yaw;
        }

        public void setYaw(double yaw) {
            this.yaw = yaw;
        }

        public double getPitch() {
            return pitch;
        }

        public void setPitch(double pitch) {
            this.pitch = pitch;
        }

        public double getRoll() {
            return roll;
        }

        public void setRoll(double roll) {
            this.roll = roll;
        }

        public Landmark[] getLandmark() {
            return landmark;
        }

        public void setLandmark(Landmark[] landmark) {
            this.landmark = landmark;
        }
    }

    class Landmark{
        private int x;      //关键点x坐标
        private int y;      //关键点y坐标

        public int getX() {
            return x;
        }

        public void setX(int x) {
            this.x = x;
        }

        public int getY() {
            return y;
        }

        public void setY(int y) {
            this.y = y;
        }
    }
}
