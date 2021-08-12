package com.glasssix.parser;

public class Parser {
    static {
        System.loadLibrary("libparser");
    }

    private long mObject;

    public Parser() {
        init();
    }

    public void fianlize() throws Throwable {
        release();
    }

    private native void init();

    private native void release();

    public native String initPlugin(String config_file_path, String license_key);

    public native String parse(String topic, String jstr, byte[] data);
}
