package com.glasssix.parser;

//import org.springframework.stereotype.Component;

//@Component
public class Parser {
    static {
		System.loadLibrary("libParser");
	}
	
	private long mObject;
	
	private Parser() {
		init();
	}



	private static class singleton {
		private static final Parser instance = new Parser();
	}
	
	public static Parser Instance() {
		return singleton.instance;
	}
	
	private native void init();
	
	public native String initPlugin(String config_file_path);
	public native String parse(String topic, String jstr);
}