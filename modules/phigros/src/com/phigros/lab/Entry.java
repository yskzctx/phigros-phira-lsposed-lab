package com.phigros.lab;
import android.app.Application;import android.app.Activity;import android.content.Context;import android.content.Intent;import com.phigros.offline.ModuleScreens;
import de.robv.android.xposed.*;import de.robv.android.xposed.callbacks.XC_LoadPackage;
public final class Entry implements IXposedHookLoadPackage,IXposedHookZygoteInit {
 static String modulePath;static volatile String failure;
 static String connection(){if(failure!=null)return failure;if(!LabBootstrap.installed)return "尚未接入 · "+LabBootstrap.phase;try{return LabNative.isReady()?"已接入 · 独立实验存档":"等待游戏原生 Hook 就绪";}catch(UnsatisfiedLinkError e){return "模块原生库未加载";}}
 public void initZygote(StartupParam startup){modulePath=startup.modulePath;}
 public void handleLoadPackage(final XC_LoadPackage.LoadPackageParam pkg)throws Throwable{
  if(!GameProfile.TARGET.equals(pkg.packageName)||!GameProfile.TARGET.equals(pkg.processName))return;
  XC_MethodHook resultHook=new XC_MethodHook(){protected void beforeHookedMethod(MethodHookParam p){if(p.args.length==3&&p.thisObject instanceof Activity&&ModuleScreens.result((Activity)p.thisObject,(Integer)p.args[0],(Integer)p.args[1],(Intent)p.args[2]))p.setResult(null);}};
  XposedBridge.hookAllMethods(Activity.class,"onActivityResult",resultHook);
  try{Class<?> unity=XposedHelpers.findClass("com.unity3d.player.UnityPlayerActivity",pkg.classLoader);XposedBridge.hookMethod(unity.getDeclaredMethod("onActivityResult",int.class,int.class,Intent.class),resultHook);}catch(NoSuchMethodException ignored){}
  XposedHelpers.findAndHookMethod(Application.class,"attach",Context.class,new XC_MethodHook(){protected void afterHookedMethod(MethodHookParam param){
   ConnectionReport.install((Application)param.thisObject,Entry::connection);
   try{LabBootstrap.prepare((Context)param.args[0],(Application)param.thisObject,pkg.processName);}catch(Throwable error){failure=LabBootstrap.phase+"失败（"+error.getClass().getSimpleName()+"）";XposedBridge.log("PhigrosLab: "+failure);}
  }});
 }
}
