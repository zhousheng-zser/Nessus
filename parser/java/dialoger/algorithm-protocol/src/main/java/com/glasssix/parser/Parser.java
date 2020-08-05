package com.glasssix.parser;

public class Parser {
	private long mObject;

	private static volatile Parser parserInstance = null;
	
	private Parser(String libName) {
		System.loadLibrary(libName);
		init();
	}

	public static Parser initParser(String libName){
		if(parserInstance == null){
			synchronized (Parser.class){
				if(parserInstance == null){
					parserInstance = new Parser(libName);
				}
			}
		}
		return parserInstance;
	}

	
	private native void init();
	
	public native String initPlugin(String config_file_path);
	public native String parse(String topic, String jstr);
}