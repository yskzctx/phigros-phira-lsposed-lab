package com.phira.lab;
import android.app.*;import android.view.*;import android.widget.*;import android.content.res.ColorStateList;import android.graphics.Typeface;import java.lang.reflect.*;import java.util.*;
/** Host Activity dialog, local WebView editor; no system-wide overlay. */
public final class Controls {
 private static final WeakHashMap<Activity,Controls> hosts=new WeakHashMap<>();
 private static boolean presented;
 private final Activity activity;private final Window.Callback previous,proxy;private Dialog dialog;private GoalPanel panel;
 private Controls(Activity a){activity=a;Window w=a.getWindow();previous=w.getCallback();proxy=(Window.Callback)Proxy.newProxyInstance(Window.Callback.class.getClassLoader(),new Class<?>[]{Window.Callback.class},(p,m,args)->{
  if(m.getName().equals("dispatchKeyEvent")&&args!=null&&args.length==1){KeyEvent e=(KeyEvent)args[0];if(e.getKeyCode()==KeyEvent.KEYCODE_VOLUME_UP){if(e.getAction()==KeyEvent.ACTION_DOWN&&e.getRepeatCount()==0)show();return true;}if(e.getKeyCode()==KeyEvent.KEYCODE_VOLUME_DOWN&&dialog!=null){if(e.getAction()==KeyEvent.ACTION_DOWN)close();return true;}}
  try{return m.invoke(previous,args);}catch(InvocationTargetException e){throw e.getCause();}
 });w.setCallback(proxy);}
 public static void attach(Activity a){if(!hosts.containsKey(a)){Controls c=new Controls(a);hosts.put(a,c);if(!presented){presented=true;a.getWindow().getDecorView().post(c::show);}}}
 public static void hide(Activity a){Controls c=hosts.get(a);if(c!=null)c.close();}
 public static void detach(Activity a){Controls c=hosts.remove(a);if(c!=null){c.close();if(a.getWindow().getCallback()==c.proxy)a.getWindow().setCallback(c.previous);}}
 private int dp(int x){return Math.round(x*activity.getResources().getDisplayMetrics().density);}
 private void show(){if(!UsageNotice.accepted()){UsageNotice.show(activity,this::show,()->{});return;}if(dialog!=null||activity.isFinishing())return;try{
  LinearLayout shell=new LinearLayout(activity);shell.setOrientation(1);shell.setBackgroundColor(0xfff1f5f9);
  LinearLayout header=new LinearLayout(activity);header.setPadding(dp(16),dp(6),dp(12),dp(6));header.setGravity(Gravity.CENTER_VERTICAL);TextView title=new TextView(activity);title.setText("Phira · 控制面板");title.setTextSize(20);title.setTypeface(null,Typeface.BOLD);title.setTextColor(0xff17283c);header.addView(title,new LinearLayout.LayoutParams(0,-2,1));Button diagnose=new Button(activity);diagnose.setText("复制诊断");diagnose.setTextColor(0xff2158a6);diagnose.setAllCaps(false);diagnose.setBackgroundTintList(ColorStateList.valueOf(0xffeaf2ff));diagnose.setOnClickListener(v->{android.content.ClipboardManager clipboard=(android.content.ClipboardManager)activity.getSystemService(android.content.Context.CLIPBOARD_SERVICE);if(clipboard!=null){clipboard.setPrimaryClip(android.content.ClipData.newPlainText("Phira 模块实时诊断",Entry.connection()));Toast.makeText(activity,"已复制当前诊断",Toast.LENGTH_SHORT).show();}});header.addView(diagnose);Button hide=new Button(activity);hide.setText("收起");hide.setTextColor(0xff2158a6);hide.setAllCaps(false);hide.setBackgroundTintList(ColorStateList.valueOf(0xffeaf2ff));hide.setOnClickListener(v->close());header.addView(hide);shell.addView(header);
  panel=new GoalPanel(activity);shell.addView(panel.view(),new LinearLayout.LayoutParams(-1,0,1));Dialog shown=new Dialog(activity,android.R.style.Theme_Material_Light_Dialog_NoActionBar);dialog=shown;shown.setContentView(shell);
  shown.setOnDismissListener(d->{if(dialog==shown){dialog=null;GoalPanel old=panel;panel=null;if(old!=null)old.destroy();}});
  shown.setOnKeyListener((d,key,event)->{if(key==KeyEvent.KEYCODE_VOLUME_DOWN){if(event.getAction()==KeyEvent.ACTION_DOWN)close();return true;}return key==KeyEvent.KEYCODE_VOLUME_UP;});shown.show();Window w=shown.getWindow();if(w!=null)w.setLayout(Math.min(dp(620),activity.getResources().getDisplayMetrics().widthPixels-dp(24)),Math.min(dp(640),activity.getResources().getDisplayMetrics().heightPixels-dp(24)));
 }catch(Throwable e){close();Toast.makeText(activity,"控制面板无法打开："+e.getClass().getSimpleName(),Toast.LENGTH_LONG).show();}}
 private void close(){Dialog old=dialog;GoalPanel previousPanel=panel;dialog=null;panel=null;if(old!=null)old.dismiss();if(previousPanel!=null)previousPanel.destroy();}
 private Controls(){throw new AssertionError();}
}
