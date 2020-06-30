package com.glasssix.parser;

public class parser {
    static {
		System.loadLibrary("parser");
	}
	
	private long mObject;
	
	private parser() {
		init();
	}

	private static class singleton {
		private static final parser instance = new parser();
	}
	
	public static parser Instance() {
		return singleton.instance;
	}
	
	private native void init();
	
	public native void initPlugin(String config_file_path);
	public native String parse(String topic, String jstr);
}