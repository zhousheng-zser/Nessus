package com.glasssix.license;

public interface LicenseDeadlineCallback {
    void run(String message, long remainingSeconds);
}
