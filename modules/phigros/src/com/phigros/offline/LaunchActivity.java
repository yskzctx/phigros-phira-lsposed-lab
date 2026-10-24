package com.phigros.offline;
import android.content.Context;import java.io.*;import com.phigros.lab.LabNative;
/** Module session adapter: never launches/replaces the host Unity activity. */
public final class LaunchActivity {
 private static boolean chosen,autoplay;
 private static native void setAutoplay(boolean value);
 public static boolean isNativeLoaded(){return LabNative.ready();}
 public static boolean isSessionChosen(){return chosen;}
 public static boolean isAutoplay(){return autoplay;}
 private static File prefs(Context c){return new File(new File(c.getApplicationInfo().dataDir,"shared_prefs"),"com.PigeonGames.Phigros.v2.playerprefs.xml");}
 public static void prepareInitialData(Context c)throws IOException{
  File state=new File(c.getFilesDir(),"offline-session");if(!state.isDirectory()&&!state.mkdirs())throw new IOException("独立会话目录无法创建");
  File pending=new File(state,"autoplay.pending");if(pending.exists()){
   File backup=new File(state,"playerprefs.xml");if(!backup.isFile())throw new IOException("自动游玩会话备份缺失");
   try(InputStream in=new FileInputStream(backup)){copyAtomic(in,prefs(c));}
   File rollback=new File(prefs(c).getPath()+".bak");if(rollback.exists()&&!rollback.delete())throw new IOException("独立会话旧备份无法清理");
   if(!pending.delete())throw new IOException("会话恢复标记无法清理");
  }
  RksEditor.applyPending(c);autoplay=false;
 }
 public static synchronized void changeAutoplay(Context c,boolean value)throws IOException{
  if(!LabNative.ready())throw new IOException("原生接口或独立存档路径尚未就绪，未开启修改");
  File state=new File(c.getFilesDir(),"offline-session"),pending=new File(state,"autoplay.pending");
  if(value&&!pending.exists()){
   if(!state.isDirectory()&&!state.mkdirs())throw new IOException("会话目录无法创建");
   try(InputStream in=new FileInputStream(prefs(c))){copyAtomic(in,new File(state,"playerprefs.xml"));}
   try(FileOutputStream out=new FileOutputStream(pending)){out.write(1);out.getFD().sync();}
  }
  NativeControls.apply(NativeControls.load(c));setAutoplay(value);autoplay=value;
 }
 public static void beginManualSession(Context c)throws IOException{changeAutoplay(c,false);chosen=true;}
 public static void setGuestAutoplay(boolean value){autoplay=value;chosen=true;}
 public static void copyAtomic(InputStream in,File target)throws IOException{
  File parent=target.getParentFile();if(!parent.isDirectory()&&!parent.mkdirs())throw new IOException("独立目录无法创建");
  File tmp=new File(parent,target.getName()+".lab-tmp");if(java.nio.file.Files.isSymbolicLink(tmp.toPath())||java.nio.file.Files.isSymbolicLink(target.toPath()))throw new IOException("独立存档不能为链接");
  try{try(FileOutputStream out=new FileOutputStream(tmp)){byte[] b=new byte[65536];int n;long size=0;while((n=in.read(b))!=-1){size+=n;if(size>16L*1024*1024)throw new IOException("独立存档过大");out.write(b,0,n);}out.getFD().sync();}
   java.nio.file.Files.move(tmp.toPath(),target.toPath(),java.nio.file.StandardCopyOption.REPLACE_EXISTING,java.nio.file.StandardCopyOption.ATOMIC_MOVE);
  }finally{java.nio.file.Files.deleteIfExists(tmp.toPath());}
 }
 private LaunchActivity(){}
}
