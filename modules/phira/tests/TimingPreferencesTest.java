package com.phira.lab;
import android.content.*;
public class TimingPreferencesTest {public static void main(String[] args){
 Context first=new Context();GoalControls.save(first,ControlsConfig.parse(2,"","90.25",""),205);
 if(GoalControls.timing(first)!=205||LabNative.lastTiming!=205)throw new AssertionError("save");
 LabNative.lastTiming=-1;Context reopened=new Context();GoalControls.apply(GoalControls.load(reopened),GoalControls.timing(reopened));if(LabNative.lastTiming!=205||GoalControls.load(reopened).mode!=2)throw new AssertionError("cold configuration restore");
 GoalControls.save(reopened,ControlsConfig.parse(0,"1000000","100",""),0);if(GoalControls.timing(new Context())!=0||LabNative.lastTiming!=0)throw new AssertionError("reset");
 first.getSharedPreferences("phira-lab-controls-082",0).edit().putInt("timing_tenths_ms",999).commit();if(GoalControls.timing(first)!=0)throw new AssertionError("stored bounds");
 System.out.println("PASS: actual GoalControls save/reopen/apply/reset and JNI timing argument using preferences/native adapters; not Android device restart");
}}
