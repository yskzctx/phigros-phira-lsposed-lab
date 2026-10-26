package com.phira.lab;
public final class LabNative {
 public static native boolean arm();public static native void disarm();public static native boolean setEnabled(boolean value);public static native int[] status();
 public static native boolean setTemporaryRoot(String directory);
 public static native boolean configure(int mode,int target,int accuracy,int timingTenthsMs,int[] rules);
 public static native int[] controlStatus();public static native String controlDetail();
 public static int[] safeStatus(){try{return status();}catch(UnsatisfiedLinkError ignored){return new int[]{0,0,0,0,0,-1,0};}}
 private LabNative(){}
}
