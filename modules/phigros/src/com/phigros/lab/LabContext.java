package com.phigros.lab;
import android.content.*;import android.content.pm.ApplicationInfo;import android.content.res.AssetManager;import java.io.*;
import de.robv.android.xposed.XposedHelpers;
/** Isolated injected tools; the host's ApplicationInfo object is never mutated. */
public final class LabContext extends ContextWrapper {
 private final File root;private final AssetManager assets;
 public LabContext(Context host,File root,AssetManager assets){super(host);this.root=root;this.assets=assets;}
 public static Context wrap(Context host){return LabBootstrap.root==null?host:new LabContext(host,LabBootstrap.root,LabBootstrap.assets);}
 @Override public File getFilesDir(){return new File(root,"files");}
 @Override public AssetManager getAssets(){return assets;}
 @Override public ApplicationInfo getApplicationInfo(){ApplicationInfo info=new ApplicationInfo(super.getApplicationInfo());info.dataDir=root.getAbsolutePath();return info;}
 @Override public SharedPreferences getSharedPreferences(String name,int mode){
  if(name==null||!name.matches("[A-Za-z0-9_.-]+"))throw new IllegalArgumentException("Invalid lab preference name");
  if(root.equals(new File(super.getApplicationInfo().dataDir)))return super.getSharedPreferences(name,mode);
  Context context=getBaseContext();while(context instanceof ContextWrapper){Context next=((ContextWrapper)context).getBaseContext();if(next==context)break;context=next;}
  // Actual AOSP ContextImpl.getSharedPreferences(File,int), verified for API34.
  return (SharedPreferences)XposedHelpers.callMethod(context,"getSharedPreferences",new File(new File(root,"shared_prefs"),name+".xml"),mode);
 }
}
