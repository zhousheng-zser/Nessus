package com.glasssix.license;

public interface EvaluateLicenseCallback {
    void run(boolean valid, String message, long remainingSeconds);
}
