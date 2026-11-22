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
 /**
  * 结算页强制显示（独立开关）。
  *
  * 与自动游玩、判定保护完全解耦：三项各有自己的偏好键，任意组合互不影响。
  * 开启后结算页显示配置的数值，包括真实一局不可能出现的组合。
  */
 public static final String FORCE="force_result_";
 public static final class Forced{
  public final boolean enabled;public final int score,accuracy,perfect,good,bad,miss,combo;
  public final boolean earlyLateOn;public final int early,late,symbol;
  Forced(boolean e,int s,int a,int p,int g,int b,int m,int c,boolean eOn,int early,int late,int symbol){
   enabled=e;score=s;accuracy=a;perfect=p;good=g;bad=b;miss=m;combo=c;
   earlyLateOn=eOn;this.early=early;this.late=late;this.symbol=symbol;}
 }
 public static Forced forced(Context c){
  SharedPreferences p=preferences(c);
  int symbol=clamp(p.getInt(FORCE+"symbol",0),0,2);
  return new Forced(p.getBoolean(FORCE+"enabled",false),
   clamp(p.getInt(FORCE+"score",0),0,99999999),clamp(p.getInt(FORCE+"accuracy",0),0,10000),
   clamp(p.getInt(FORCE+"perfect",0),0,999999),clamp(p.getInt(FORCE+"good",0),0,999999),
   clamp(p.getInt(FORCE+"bad",0),0,999999),clamp(p.getInt(FORCE+"miss",0),0,999999),
   clamp(p.getInt(FORCE+"combo",0),0,999999),
   p.getBoolean(FORCE+"early_late_on",false),
   clamp(p.getInt(FORCE+"early",0),0,999999),clamp(p.getInt(FORCE+"late",0),0,999999),symbol);
 }
 private static int clamp(int v,int low,int high){return v<low?low:v>high?high:v;}
 public static void applyForced(Forced v){
  if(!LabNative.configureForcedResult(v.enabled,v.score,v.accuracy,v.perfect,v.good,v.bad,v.miss,v.combo,
   v.earlyLateOn,v.early,v.late,v.symbol))
   throw new IllegalArgumentException("原生层拒绝该结算显示配置");
 }
 public static void saveForced(Context c,Forced v){
  applyForced(v);
  if(!preferences(c).edit().putBoolean(FORCE+"enabled",v.enabled).putInt(FORCE+"score",v.score)
   .putInt(FORCE+"accuracy",v.accuracy).putInt(FORCE+"perfect",v.perfect).putInt(FORCE+"good",v.good)
   .putInt(FORCE+"bad",v.bad).putInt(FORCE+"miss",v.miss).putInt(FORCE+"combo",v.combo)
   .putBoolean(FORCE+"early_late_on",v.earlyLateOn).putInt(FORCE+"early",v.early)
   .putInt(FORCE+"late",v.late).putInt(FORCE+"symbol",v.symbol).commit())
   throw new IllegalStateException("结算显示设置保存失败");
 }
 /** 启动时把已保存的配置推给原生层；失败只记录，不影响启动。 */
 public static void initializeForced(Context c){
  try{applyForced(forced(c));}catch(RuntimeException e){android.util.Log.e("PhiraLab","forced result config ignored",e);}
 }
 public static void flags(Context c,boolean block,int guard,int score){if(guard<0||guard>3||score<0||score>1000000)throw new IllegalArgumentException("未知保护");preferences(c).edit().putBoolean("block_touch",block).putInt("assist_guard",guard).putInt("guard_score",score).commit();}
 private GoalControls(){}
}
