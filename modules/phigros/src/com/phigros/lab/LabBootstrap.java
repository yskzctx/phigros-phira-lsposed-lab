package com.phigros.lab;
import android.app.*;import android.content.*;import android.content.pm.*;import android.content.res.AssetManager;import android.os.*;import java.io.*;import java.security.*;import java.util.*;import java.util.zip.*;
import com.phigros.offline.*;
import de.robv.android.xposed.*;
public final class LabBootstrap {
 static File root;static AssetManager assets;static boolean installed,nativeLoaded,skinInitialized;static volatile String phase="等待游戏";
 private static String hash(InputStream in)throws Exception{MessageDigest md=MessageDigest.getInstance("SHA-256");byte[] b=new byte[262144];int n;while((n=in.read(b))!=-1)md.update(b,0,n);StringBuilder out=new StringBuilder();for(byte v:md.digest())out.append(String.format(Locale.ROOT,"%02x",v&255));return out.toString();}
 private static String nativeHash(ApplicationInfo info)throws Exception{
  File extracted=new File(info.nativeLibraryDir,"libil2cpp.so");if(extracted.isFile())try(InputStream in=new FileInputStream(extracted)){return hash(in);}
  ArrayList<String> paths=new ArrayList<>();paths.add(info.sourceDir);if(info.splitSourceDirs!=null)Collections.addAll(paths,info.splitSourceDirs);
  for(String path:paths)try(ZipFile zip=new ZipFile(path)){ZipEntry entry=zip.getEntry("lib/arm64-v8a/libil2cpp.so");if(entry!=null)try(InputStream in=zip.getInputStream(entry)){return hash(in);}}
  return "";
 }
 public static void prepare(Context host,Application app,String process)throws Exception{
  if(installed)return;
  phase="核对游戏版本与原生库";
  PackageInfo info=host.getPackageManager().getPackageInfo(host.getPackageName(),0);long version=Build.VERSION.SDK_INT>=28?info.getLongVersionCode():info.versionCode;
  String abi=Arrays.asList(Build.SUPPORTED_ABIS).contains("arm64-v8a")?"arm64-v8a":"unsupported";
  if(!android.os.Process.is64Bit()||!GameProfile.supports(host.getPackageName(),process,version,abi,nativeHash(host.getApplicationInfo())))throw new IOException("仅适配原版4.0.1(157) arm64及已核对的原生库；本次未启用修改");
  phase="读取模块资源";try{Context module=host.createPackageContext(GameProfile.MODULE,0);assets=module.getAssets();}
  catch(PackageManager.NameNotFoundException hidden){
   // The unchanged host manifest cannot query new modules on Android 11+.
   // Use the module path supplied by Xposed, not a guessed /data/app path.
   if(Entry.modulePath==null)throw hidden;
   assets=android.content.res.XModuleResources.createInstance(Entry.modulePath,null).getAssets();
  }
  phase="备份原版存档";root=ShadowStore.prepareOriginal(new File(host.getApplicationInfo().dataDir));
  Context original=LabContext.wrap(host);LaunchActivity.prepareInitialData(original);
  phase="加载控制原生库";System.loadLibrary("phigros_lsp");nativeLoaded=true;if(!LabNative.setRoot(new File(root,"files").getCanonicalPath()))throw new IOException("控制准备参数被拒绝");
  installed=true;phase="等待游戏原生 Hook";
 }
 static final Handler handler=new Handler(Looper.getMainLooper());
 static final WeakHashMap<Activity,Runnable> retries=new WeakHashMap<>();
 public static void watch(Activity a){
  stopWatching(a);if(!"com.unity3d.player.UnityPlayerActivity".equals(a.getClass().getName()))return;
  UsageNotice.show(a,()->{if(installed)OverlayMenu.resume(a);else new AlertDialog.Builder(new android.view.ContextThemeWrapper(a,android.R.style.Theme_Material_Light_Dialog_Alert)).setTitle("Phigros 模块未接入").setMessage(Entry.connection()).setPositiveButton("复制诊断",(d,w)->{((android.content.ClipboardManager)a.getSystemService(Context.CLIPBOARD_SERVICE)).setPrimaryClip(android.content.ClipData.newPlainText("Phigros 模块连接报告",Entry.connection()));}).setNegativeButton("继续原版游戏",null).show();},()->{});
  Runnable retry=new Runnable(){int attempts;public void run(){
   if(a.isFinishing()||a.isDestroyed()){stopWatching(a);return;}
   if(installed&&LabNative.ready()){
    try{Context c=LabContext.wrap(a);if(!LaunchActivity.isSessionChosen())LaunchActivity.beginManualSession(c);if(!skinInitialized){SkinController.initialize(c);skinInitialized=true;}phase="原版存档控制已就绪";}
    catch(Throwable e){Entry.failure="控制初始化失败（"+e.getClass().getSimpleName()+"）";}
    ConnectionReport.send(a,Entry.connection());retries.remove(a);return;
   }
   if(++attempts<120)handler.postDelayed(this,250);else{ConnectionReport.send(a,Entry.connection());retries.remove(a);}
  }};retries.put(a,retry);handler.post(retry);
 }
 public static void stopWatching(Activity a){Runnable pending=retries.remove(a);if(pending!=null)handler.removeCallbacks(pending);}
 private LabBootstrap(){}
}
