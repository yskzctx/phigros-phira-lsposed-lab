package com.phigros.lab;
import android.app.Application;import android.app.Activity;import android.content.Context;import android.content.Intent;import com.phigros.offline.ModuleScreens;
import de.robv.android.xposed.*;import de.robv.android.xposed.callbacks.XC_LoadPackage;
public final class Entry implements IXposedHookLoadPackage,IXposedHookZygoteInit {
 static String modulePath;static volatile String failure;
 public static String connection(){if(failure!=null)return failure;if(!LabBootstrap.installed)return "尚未接入 · "+LabBootstrap.phase;try{int[] d=LabNative.diagnostics();return (LabNative.isReady()?"已接入 · 原版存档（已备份）":d[0]==0?"LSPosed 尚未提供原生 Hook 接口":d[2]==0?"等待 Unity 加载游戏原生库":d[3]<0?"原生接入失败":"正在安装游戏判定控制")+" · API="+d[0]+" 准备="+d[1]+" 库="+d[2]+" 阶段="+d[3]+" Hook="+d[4]+" 错误="+d[5];}catch(UnsatisfiedLinkError e){return "模块原生库未加载";}}

 public void initZygote(StartupParam startup){modulePath=startup.modulePath;}
 public void handleLoadPackage(final XC_LoadPackage.LoadPackageParam pkg)throws Throwable{
  if(!GameProfile.TARGET.equals(pkg.packageName)||!GameProfile.TARGET.equals(pkg.processName))return;
  XC_MethodHook resultHook=new XC_MethodHook(){protected void beforeHookedMethod(MethodHookParam p){if(p.args.length==3&&p.thisObject instanceof Activity&&ModuleScreens.result((Activity)p.thisObject,(Integer)p.args[0],(Integer)p.args[1],(Intent)p.args[2]))p.setResult(null);}};
  XposedBridge.hookAllMethods(Activity.class,"onActivityResult",resultHook);
  try{Class<?> unity=XposedHelpers.findClass("com.unity3d.player.UnityPlayerActivity",pkg.classLoader);XposedBridge.hookMethod(unity.getDeclaredMethod("onActivityResult",int.class,int.class,Intent.class),resultHook);}catch(Throwable optionalResultHook){XposedBridge.log("PhigrosLab: optional Unity result override not present; Activity fallback retained");}
  XposedHelpers.findAndHookMethod(Application.class,"attach",Context.class,new XC_MethodHook(){protected void afterHookedMethod(MethodHookParam param){
   ConnectionReport.install((Application)param.thisObject,Entry::connection);
   ((Application)param.thisObject).registerActivityLifecycleCallbacks(new Application.ActivityLifecycleCallbacks(){
    public void onActivityResumed(Activity a){LabBootstrap.watch(a);}public void onActivityPaused(Activity a){LabBootstrap.stopWatching(a);com.phigros.offline.OverlayMenu.pause(a);}public void onActivityDestroyed(Activity a){LabBootstrap.stopWatching(a);com.phigros.offline.OverlayMenu.destroy(a);}
    public void onActivityCreated(Activity a,android.os.Bundle b){}public void onActivityStarted(Activity a){}public void onActivityStopped(Activity a){}public void onActivitySaveInstanceState(Activity a,android.os.Bundle b){}
   });
   try{LabBootstrap.prepare((Context)param.args[0],(Application)param.thisObject,pkg.processName);}catch(Throwable error){failure=LabBootstrap.phase+"失败（"+error.getClass().getSimpleName()+"）："+String.valueOf(error.getMessage());XposedBridge.log("PhigrosLab: "+LabBootstrap.phase+" failed: "+error.getClass().getSimpleName());}
  }});
 }
}
