package com.phira.lab;
import android.content.Context;import android.content.SharedPreferences;import java.math.BigDecimal;
public final class GoalControls {
 public static SharedPreferences preferences(Context c){return c.getSharedPreferences("phira-lab-controls-082",0);}
 public static ControlsConfig load(Context c){SharedPreferences p=preferences(c);try{return ControlsConfig.parse(p.getInt("goal_mode",0),p.getString("score","1000000"),p.getString("accuracy","100"),p.getString("rules",""));}catch(RuntimeException e){return ControlsConfig.parse(0,"1000000","100","");}}
 public static int timing(Context c){int t=preferences(c).getInt("timing_tenths_ms",0);return t>=0&&t<=750?t:0;}
 public static boolean blockTouch(Context c){return preferences(c).getBoolean("block_touch",true);}
 public static int assistGuard(Context c){int v=preferences(c).getInt("assist_guard",0);return v>=0&&v<=3?v:0;}
 public static int guardScore(Context c){int v=preferences(c).getInt("guard_score",0);return v>=0&&v<=1000000?v:0;}
 public static void apply(ControlsConfig c){apply(c,0);}
 public static void apply(ControlsConfig c,int timing){apply(c,timing,true,0,0);}
 public static void apply(ControlsConfig c,int timing,boolean blockTouch,int assistGuard,int guardScore){
  if(assistGuard<0||assistGuard>3||guardScore<0||guardScore>1000000)throw new IllegalArgumentException("成绩保护参数无效");
  if(!LabNative.configure(c.mode,c.target,c.accuracy,timing,blockTouch,assistGuard,guardScore,c.packed))throw new IllegalArgumentException("原生判定控制拒绝此配置");
 }
 public static void save(Context context,ControlsConfig c){save(context,c,timing(context));}
 public static void save(Context context,ControlsConfig c,int timing){save(context,c,timing,blockTouch(context),assistGuard(context),guardScore(context));}
 public static void save(Context context,ControlsConfig c,int timing,boolean blockTouch,int assistGuard,int guardScore){
  apply(c,timing,blockTouch,assistGuard,guardScore);
  if(!preferences(context).edit().putInt("goal_mode",c.mode).putString("score",Integer.toString(c.target)).putString("accuracy",new BigDecimal(c.accuracy).divide(new BigDecimal(100)).toPlainString()).putString("rules",c.encodedRules).putInt("timing_tenths_ms",timing).putBoolean("block_touch",blockTouch).putInt("assist_guard",assistGuard).putInt("guard_score",guardScore).commit())throw new IllegalStateException("配置未能写入存储");
 }
 public static int[] status(){try{int[] s=LabNative.controlStatus();return s!=null&&s.length==21?s:new int[21];}catch(UnsatisfiedLinkError e){return new int[21];}}
 public static void flags(Context c,boolean block,int guard,int score){if(guard<0||guard>3||score<0||score>1000000)throw new IllegalArgumentException("未知保护");preferences(c).edit().putBoolean("block_touch",block).putInt("assist_guard",guard).putInt("guard_score",score).commit();}
 private GoalControls(){}
}
