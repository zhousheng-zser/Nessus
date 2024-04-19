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

    public native void initPlugin(String config_file_path, String license_key) throws Throwable;
	
	public native String createInstance(String qualified_name, String param) throws Throwable;
	
    public native void releaseInstance(String instance_id);

    public native String execute(String instance_id, String param, byte[] imgData, int height, int width, int img_format, boolean is_base64, byte[] outputData) throws Throwable;
}
