package com.phigros.lab;
public final class LabNative {
 public static native boolean setRoot(String path);
 public static native boolean isReady();
 public static native boolean readyOrAbort();
 public static native int[] diagnostics();
 public static boolean ready(){try{return isReady();}catch(UnsatisfiedLinkError error){return false;}}
 private LabNative(){}
}
