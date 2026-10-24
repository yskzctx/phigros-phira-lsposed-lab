package com.phigros.lab;
import android.app.*;import android.content.*;import android.content.pm.*;import android.content.res.AssetManager;import android.os.*;import java.io.*;import java.security.*;import java.util.*;import java.util.zip.*;
import com.phigros.offline.*;
import de.robv.android.xposed.*;
public final class LabBootstrap {
 static File root;static AssetManager assets;static boolean installed,nativeLoaded,earlyPreferences,skinInitialized;
 private static String hash(InputStream in)throws Exception{MessageDigest md=MessageDigest.getInstance("SHA-256");byte[] b=new byte[262144];int n;while((n=in.read(b))!=-1)md.update(b,0,n);StringBuilder out=new StringBuilder();for(byte v:md.digest())out.append(String.format(Locale.ROOT,"%02x",v&255));return out.toString();}
 private static String nativeHash(ApplicationInfo info)throws Exception{
  File extracted=new File(info.nativeLibraryDir,"libil2cpp.so");if(extracted.isFile())try(InputStream in=new FileInputStream(extracted)){return hash(in);}
  ArrayList<String> paths=new ArrayList<>();paths.add(info.sourceDir);if(info.splitSourceDirs!=null)Collections.addAll(paths,info.splitSourceDirs);
  for(String path:paths)try(ZipFile zip=new ZipFile(path)){ZipEntry entry=zip.getEntry("lib/arm64-v8a/libil2cpp.so");if(entry!=null)try(InputStream in=zip.getInputStream(entry)){return hash(in);}}
  return "";
 }
 public static void prepare(Context host,Application app,String process)throws Exception{
  if(installed)return;
  PackageInfo info=host.getPackageManager().getPackageInfo(host.getPackageName(),0);long version=Build.VERSION.SDK_INT>=28?info.getLongVersionCode():info.versionCode;
  String abi=Arrays.asList(Build.SUPPORTED_ABIS).contains("arm64-v8a")?"arm64-v8a":"unsupported";
  if(!GameProfile.supports(host.getPackageName(),process,version,abi,nativeHash(host.getApplicationInfo())))throw new IOException("仅适配原版4.0.1(157) arm64及已核对的原生库；本次未启用修改");
  try{Context module=host.createPackageContext(GameProfile.MODULE,0);assets=module.getAssets();}
  catch(PackageManager.NameNotFoundException hidden){
   // The unchanged host manifest cannot query new modules on Android 11+.
   // Use the module path supplied by Xposed, not a guessed /data/app path.
   if(Entry.modulePath==null)throw hidden;
   assets=android.content.res.XModuleResources.createInstance(Entry.modulePath,null).getAssets();
  }
  try(InputStream seed=assets.open("offline/all-ap.xml")){root=ShadowStore.prepare(new File(host.getApplicationInfo().dataDir),seed);}
  Context isolated=LabContext.wrap(host);LaunchActivity.prepareInitialData(isolated);
  XposedHelpers.findAndHookMethod("android.app.ContextImpl",null,"getSharedPreferences",File.class,int.class,new XC_MethodHook(){protected void beforeHookedMethod(MethodHookParam param)throws Throwable{
   File requested=(File)param.args[0];File original=new File(new File(host.getApplicationInfo().dataDir,"shared_prefs"),GameProfile.TARGET+".v2.playerprefs.xml");
   if(requested==null||!requested.getCanonicalFile().equals(original.getCanonicalFile()))return;
   if(!nativeLoaded||!LabNative.readyOrAbort()){earlyPreferences=true;return;}
   param.args[0]=new File(root,"shared_prefs/"+original.getName());
  }});
  System.loadLibrary("phigros_lsp");nativeLoaded=true;if(earlyPreferences||!LabNative.setRoot(new File(root,"files").getCanonicalPath()))throw new IOException("存档已被提前读取或独立路径无法启用；本次不加载修改");
  app.registerActivityLifecycleCallbacks(new Application.ActivityLifecycleCallbacks(){
   public void onActivityResumed(Activity a){if(!LabNative.isReady())return;try{Context c=LabContext.wrap(a);if(!LaunchActivity.isSessionChosen())LaunchActivity.beginManualSession(c);if(!skinInitialized){SkinController.initialize(c);skinInitialized=true;}OverlayMenu.resume(a);}catch(Exception e){XposedBridge.log("PhigrosLab: local tools failed; controls remain off");}}
   public void onActivityPaused(Activity a){OverlayMenu.pause(a);}public void onActivityDestroyed(Activity a){OverlayMenu.destroy(a);}
   public void onActivityCreated(Activity a,Bundle b){}public void onActivityStarted(Activity a){}public void onActivityStopped(Activity a){}public void onActivitySaveInstanceState(Activity a,Bundle b){}
  });
  installed=true;
 }
 private LabBootstrap(){}
}
