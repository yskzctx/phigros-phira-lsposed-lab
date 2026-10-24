package com.phira.lab;
import android.app.*;import android.view.*;import android.widget.*;import android.content.res.ColorStateList;import java.lang.reflect.*;import java.util.*;import de.robv.android.xposed.XposedHelpers;
/** A normal Activity dialog; no system-wide overlay and no WebView network surface. */
public final class Controls {
 private static final WeakHashMap<Activity,Controls> hosts=new WeakHashMap<>();
 private final Activity activity;private final Window.Callback previous,proxy;private Dialog dialog;private int page;
 private Controls(Activity a){activity=a;Window w=a.getWindow();previous=w.getCallback();proxy=(Window.Callback)Proxy.newProxyInstance(Window.Callback.class.getClassLoader(),new Class<?>[]{Window.Callback.class},(p,m,args)->{
  if(m.getName().equals("dispatchKeyEvent")&&args!=null&&args.length==1){KeyEvent e=(KeyEvent)args[0];if(e.getKeyCode()==KeyEvent.KEYCODE_VOLUME_UP){if(e.getAction()==KeyEvent.ACTION_DOWN&&e.getRepeatCount()==0)show();return true;}if(e.getKeyCode()==KeyEvent.KEYCODE_VOLUME_DOWN&&dialog!=null){if(e.getAction()==KeyEvent.ACTION_DOWN)close();return true;}}
  try{return m.invoke(previous,args);}catch(InvocationTargetException e){throw e.getCause();}
 });w.setCallback(proxy);}
 public static void attach(Activity a){if(!hosts.containsKey(a))hosts.put(a,new Controls(a));}
 public static void hide(Activity a){Controls c=hosts.get(a);if(c!=null)c.close();}
 public static void detach(Activity a){Controls c=hosts.remove(a);if(c!=null){c.close();if(a.getWindow().getCallback()==c.proxy)a.getWindow().setCallback(c.previous);}}
 private int dp(int x){return Math.round(x*activity.getResources().getDisplayMetrics().density);}
 private TextView text(String value,int size){TextView t=new TextView(activity);t.setText(value);t.setTextSize(size);t.setTextColor(0xff17283c);t.setPadding(0,dp(8),0,dp(8));return t;}
 private void button(LinearLayout box,String label,Runnable action){Button b=new Button(activity);b.setText(label);b.setAllCaps(false);b.setTextColor(0xff2158a6);b.setBackgroundTintList(ColorStateList.valueOf(0xffeaf2ff));b.setMinHeight(dp(48));b.setOnClickListener(v->action.run());box.addView(b,new LinearLayout.LayoutParams(-1,-2));}
 private void message(String s){Toast.makeText(activity,s,Toast.LENGTH_LONG).show();}
 private void show(){if(dialog!=null||activity.isFinishing())return;
  int[] status=LabNative.safeStatus();boolean active=Entry.redirected&&status[0]==1&&status[1]==1;
  LinearLayout box=new LinearLayout(activity);box.setOrientation(1);box.setPadding(dp(20),dp(10),dp(20),dp(10));box.setBackgroundColor(0xfff1f5f9);
  LinearLayout header=new LinearLayout(activity);header.setGravity(Gravity.CENTER_VERTICAL);TextView title=text("Phira · 本地实验",22);title.setTypeface(null,android.graphics.Typeface.BOLD);header.addView(title,new LinearLayout.LayoutParams(0,-2,1));Button hide=new Button(activity);hide.setText("收起");hide.setTextColor(0xff2158a6);hide.setBackgroundTintList(ColorStateList.valueOf(0xffeaf2ff));hide.setOnClickListener(v->close());header.addView(hide);box.addView(header);
  RadioGroup tabs=new RadioGroup(activity);tabs.setOrientation(0);String[] labels={"游玩","历史","连接"};for(int i=0;i<3;i++){RadioButton t=new RadioButton(activity);t.setId(100+i);t.setText(labels[i]);t.setTextColor(0xff2158a6);t.setMinHeight(dp(48));tabs.addView(t,new RadioGroup.LayoutParams(0,-2,1));}tabs.check(100+page);tabs.setOnCheckedChangeListener((g,id)->{page=id-100;close();show();});box.addView(tabs);
  if(page==0){
  box.addView(text(active?"已接入 · 独立实验存档":"尚未接通，请查看「连接」",14));
  Switch autoplay=new Switch(activity);autoplay.setText("原版自动游玩 · 标准 AP");autoplay.setTextSize(18);autoplay.setTextColor(0xff17283c);autoplay.setChecked(status[2]==1);autoplay.setEnabled(active);autoplay.setMinHeight(dp(56));box.addView(autoplay);
  autoplay.setOnCheckedChangeListener((view,value)->{autoplay.setEnabled(false);new Thread(()->{String error=null;try{if(value)LabStore.beginAutoplay(Entry.root);if(!LabNative.setEnabled(value))throw new IllegalStateException("原生接口未就绪");}catch(Exception e){error=e.getMessage();}final String result=error;activity.runOnUiThread(()->{close();if(result!=null)message(result);show();});},"PhiraLabToggle").start();});
  box.addView(text("自动游玩冷启动默认关闭。开启后本次会话按不计成绩处理；重启时只恢复历史记录，保留新导入的谱面与皮肤。",14));
  box.addView(text("目标分数、目标准确率、逐音符规则及 RKS 尚未接入；本候选未达到修改版功能范围。",14));
  }else if(page==1){
  box.addView(text("实验历史记录",20));box.addView(text("仅修改独立实验存档，完全退出并冷启动后生效。原版存档保持不动。",14));
  button(box,"历史记录：全部 AP（重启生效）",()->history("all-ap"));button(box,"恢复修改前的历史记录（重启生效）",()->history("restore"));
  box.addView(text("谱面与皮肤请使用 Phira 原有页面管理。模块不重复提供导入入口。",14));
  }else{box.addView(text(Entry.connection(),20));box.addView(text("启动阶段："+Entry.phase+"\n独立存档："+(Entry.redirected?"已切换":"未切换")+"\n原生帧数："+status[4],14));button(box,"刷新状态",()->{close();show();});box.addView(text("版本或函数特征不匹配时保持关闭，不强行套用偏移。",14));}
  ScrollView scroll=new ScrollView(activity);scroll.addView(box);dialog=new Dialog(activity,android.R.style.Theme_Material_Light_Dialog_NoActionBar);dialog.setContentView(scroll);dialog.setOnDismissListener(d->{if(dialog==d)dialog=null;});dialog.setOnKeyListener((d,key,event)->{if(key==KeyEvent.KEYCODE_VOLUME_DOWN){if(event.getAction()==KeyEvent.ACTION_DOWN)close();return true;}if(key==KeyEvent.KEYCODE_VOLUME_UP)return true;return false;});dialog.show();Window w=dialog.getWindow();if(w!=null)w.setLayout(Math.min(dp(520),activity.getResources().getDisplayMetrics().widthPixels-dp(28)),Math.min(dp(590),activity.getResources().getDisplayMetrics().heightPixels-dp(28)));
 }
 private void history(String operation){try{if(!Entry.redirected||Entry.root==null)throw new IllegalStateException("实验路径尚未启用");LabStore.queueHistory(Entry.root,operation);message("已排队，完全退出并冷启动 Phira 后生效；仅修改实验记录");}catch(Exception e){message(e.getMessage());}}
 private void close(){Dialog d=dialog;dialog=null;if(d!=null)d.dismiss();}
 private Controls(){throw new AssertionError();}
}
