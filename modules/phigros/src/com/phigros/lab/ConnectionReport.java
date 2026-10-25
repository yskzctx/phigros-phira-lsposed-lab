package com.phigros.lab;
import android.app.*;import android.content.*;import android.os.*;import java.util.function.Supplier;
/** An explicit one-shot capability returns only connection state to our private receiver. */
public final class ConnectionReport extends BroadcastReceiver {
 public static final String MODULE="com.phigros.lab.lsp", EXTRA="com.phigros.lab.lsp.connection";
 public void onReceive(Context c,Intent i){String s=i.getStringExtra("state");if(s==null||s.length()>512)return;c.getSharedPreferences("connection",0).edit().putString("state",s).putLong("received",System.currentTimeMillis()).apply();}
 public static void install(Application app,Supplier<String> state){app.registerActivityLifecycleCallbacks(new Application.ActivityLifecycleCallbacks(){
  public void onActivityResumed(Activity a){new Handler(Looper.getMainLooper()).postDelayed(()->send(a,state.get()),600);}
  public void onActivityCreated(Activity a,Bundle b){}public void onActivityStarted(Activity a){}public void onActivityPaused(Activity a){}public void onActivityStopped(Activity a){}public void onActivitySaveInstanceState(Activity a,Bundle b){}public void onActivityDestroyed(Activity a){}
 });}
 public static void send(Activity a,String state){try{PendingIntent p=a.getIntent().getParcelableExtra(EXTRA);if(p==null||!MODULE.equals(p.getCreatorPackage()))return;Intent fill=new Intent();fill.putExtra("state",state);p.send(a,0,fill);}catch(PendingIntent.CanceledException|RuntimeException ignored){}}
}
