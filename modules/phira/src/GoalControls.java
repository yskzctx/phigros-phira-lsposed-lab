package com.phira.lab;
import android.content.Context;import android.content.SharedPreferences;import java.math.BigDecimal;
public final class GoalControls {
 public static SharedPreferences preferences(Context c){return c.getSharedPreferences("phira-lab-controls-082",0);}
 public static ControlsConfig load(Context c){SharedPreferences p=preferences(c);try{return ControlsConfig.parse(p.getInt("goal_mode",0),p.getString("score","1000000"),p.getString("accuracy","100"),p.getString("rules",""));}catch(RuntimeException e){return ControlsConfig.parse(0,"1000000","100","");}}
 public static void apply(ControlsConfig c){if(!LabNative.configure(c.mode,c.target,c.accuracy,c.packed))throw new IllegalArgumentException("原生判定控制拒绝此配置");}
 public static void save(Context context,ControlsConfig c){apply(c);if(!preferences(context).edit().putInt("goal_mode",c.mode).putString("score",Integer.toString(c.target)).putString("accuracy",new BigDecimal(c.accuracy).divide(new BigDecimal(100)).toPlainString()).putString("rules",c.encodedRules).commit())throw new IllegalStateException("配置未能写入存储");}
 public static int[] status(){try{int[] s=LabNative.controlStatus();return s!=null&&s.length==21?s:new int[21];}catch(UnsatisfiedLinkError e){return new int[21];}}
 private GoalControls(){}
}
