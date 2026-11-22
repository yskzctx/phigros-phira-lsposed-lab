package com.phira.lab;
public final class LabNative {
 public static native boolean arm();public static native void disarm();public static native boolean setEnabled(boolean value);public static native int[] status();
 public static native boolean setTemporaryRoot(String directory);
 public static native boolean configure(int mode,int target,int accuracy,int timingTenthsMs,boolean blockTouch,int assistGuard,int guardScore,int[] rules);
 /** 结算页强制显示：独立开关，与自动游玩、判定保护互不影响。 */
 public static native boolean configureForcedResult(boolean enabled,int score,int accuracy,int perfect,int good,int bad,int miss,int combo,boolean earlyLateOn,int early,int late,int symbol);
 public static native int[] controlStatus();public static native String controlDetail();
 public static int[] safeStatus(){try{return status();}catch(UnsatisfiedLinkError ignored){return new int[]{0,0,0,0,0,-1,0};}}
 private LabNative(){}
}
