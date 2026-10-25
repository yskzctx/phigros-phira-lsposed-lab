package com.phigros.lab;
import java.io.*;import java.nio.file.*;

/** Private lab storage. The official preferences are opened read-only for backup. */
public final class ShadowStore {
 private static final String PREF="com.PigeonGames.Phigros.v2.playerprefs.xml";
 private static File directory(File parent,String name)throws IOException{
  File dir=new File(parent,name);if(Files.isSymbolicLink(dir.toPath())||!dir.getCanonicalFile().equals(dir.getAbsoluteFile()))throw new IOException("Lab directory is linked");
  if(!dir.isDirectory()&&!dir.mkdir())throw new IOException("Cannot create lab directory");return dir;
 }
 private static void regular(File path)throws IOException{
  if(Files.isSymbolicLink(path.toPath())||!path.getCanonicalFile().equals(path.getAbsoluteFile())||!path.isFile())throw new IOException("Unexpected lab file");
 }
 public static synchronized File prepareOriginal(File applicationData)throws IOException{
  File app=applicationData.getCanonicalFile();if(!app.isDirectory())throw new IOException("Application data missing");
  File files=directory(app,"files"),backups=directory(files,"phigros-module-backups");
  File original=new File(new File(app,"shared_prefs"),PREF),backup=new File(backups,"original-playerprefs-before-module.xml"),absent=new File(backups,"original-playerprefs-absent-v1");
  if(backup.exists()){regular(backup);return app;}
  if(absent.exists()){regular(absent);if(!original.exists())return app;}
  if(original.exists()){regular(original);try(InputStream in=new FileInputStream(original)){writeAtomic(in,backup,16L*1024*1024);}}
  else try(InputStream in=new ByteArrayInputStream(new byte[]{1})){writeAtomic(in,absent,1);}
  return app;
 }
 public static synchronized File prepare(File applicationData,InputStream anonymousSeed)throws IOException{
  File app=applicationData.getCanonicalFile();if(!app.isDirectory())throw new IOException("Application data missing");
  File files=directory(app,"files"),lab=directory(files,"phigros-lab-v157"),prefs=directory(lab,"shared_prefs");directory(lab,"files");
  File seedMarker=new File(lab,"seeded-v1"),shadow=new File(prefs,PREF),backup=new File(lab,"original-backup.xml"),absent=new File(lab,"original-absent-v1");
  if(seedMarker.exists()){regular(seedMarker);regular(shadow);if(backup.exists())regular(backup);else regular(absent);return lab;}
  File original=new File(new File(app,"shared_prefs"),PREF);
  if(original.exists()){
   regular(original);if(!backup.exists())try(InputStream in=new FileInputStream(original)){writeAtomic(in,backup,16L*1024*1024);}else regular(backup);
  }else if(!absent.exists()){try(InputStream in=new ByteArrayInputStream(new byte[]{1})){writeAtomic(in,absent,1);}}
  writeAtomic(anonymousSeed,shadow,16L*1024*1024);
  try(InputStream in=new ByteArrayInputStream(new byte[]{1})){writeAtomic(in,seedMarker,1);}
  return lab;
 }
 private static void writeAtomic(InputStream input,File destination,long limit)throws IOException{
  if(Files.isSymbolicLink(destination.toPath())||!destination.getAbsoluteFile().equals(destination.getCanonicalFile()))throw new IOException("Lab target is linked");
  File temp=new File(destination.getParentFile(),destination.getName()+".pending");if(Files.isSymbolicLink(temp.toPath()))throw new IOException("Lab temporary file is linked");
  try{
   try(FileOutputStream out=new FileOutputStream(temp)){byte[] buf=new byte[65536];long total=0;int n;while((n=input.read(buf))!=-1){total+=n;if(total>limit)throw new IOException("Lab file too large");out.write(buf,0,n);}out.getFD().sync();}
   try{Files.move(temp.toPath(),destination.toPath(),StandardCopyOption.ATOMIC_MOVE,StandardCopyOption.REPLACE_EXISTING);}
   catch(AtomicMoveNotSupportedException e){Files.move(temp.toPath(),destination.toPath(),StandardCopyOption.REPLACE_EXISTING);}
  }finally{Files.deleteIfExists(temp.toPath());}
 }
 private ShadowStore(){}
}
