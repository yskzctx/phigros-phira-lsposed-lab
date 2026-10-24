package com.phigros.lab;
import java.io.*;import java.nio.file.*;import java.util.*;
public final class ShadowStoreTest{
 public static void main(String[] args)throws Exception{
  File root=Files.createTempDirectory("phigros-shadow-").toFile();File shared=new File(root,"shared_prefs");shared.mkdir();File original=new File(shared,"com.PigeonGames.Phigros.v2.playerprefs.xml");byte[] privateSave="original private save".getBytes("UTF-8");Files.write(original.toPath(),privateSave);
  File lab=ShadowStore.prepare(root,new ByteArrayInputStream("local anonymous AP seed".getBytes("UTF-8")));
  if(!Arrays.equals(Files.readAllBytes(original.toPath()),privateSave))throw new AssertionError("original altered");
  if(!Arrays.equals(Files.readAllBytes(new File(lab,"original-backup.xml").toPath()),privateSave))throw new AssertionError("backup altered");
  File pref=new File(lab,"shared_prefs/com.PigeonGames.Phigros.v2.playerprefs.xml");if(!pref.isFile())throw new AssertionError("shadow not initialized");
  Files.write(pref.toPath(),"user lab progress".getBytes("UTF-8"));ShadowStore.prepare(root,new ByteArrayInputStream("wrong second seed".getBytes("UTF-8")));
  if(!new String(Files.readAllBytes(pref.toPath()),"UTF-8").equals("user lab progress"))throw new AssertionError("lab overwritten");
  File another=Files.createTempDirectory("phigros-shadow-failure-").toFile();try{ShadowStore.prepare(another,new InputStream(){public int read()throws IOException{throw new IOException("broken input");}});throw new AssertionError("failure hidden");}catch(IOException expected){}
  if(new File(another,"files/phigros-lab-v157/seeded-v1").exists())throw new AssertionError("partial marked ready");
  System.out.println("Shadow store: original intact, private backup, independent seed, persisted lab state and failure handling passed");
 }
}
