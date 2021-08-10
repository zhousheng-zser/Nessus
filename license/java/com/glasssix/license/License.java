package com.glasssix.license;

public class License {
    static {
        System.loadLibrary("liblicense_client");
    }

    public static native void initSystem(String licenseKey);
    public static native void evaluate(EvaluateLicenseCallback callback);
    public static native void requestAsync(RequestLicenseAsyncCallback callabck);
    public static native void setDeadlineCallback(LicenseDeadlineCallback deadlineCallback);
}
