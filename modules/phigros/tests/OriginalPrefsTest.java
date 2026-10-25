package com.phigros.lab;
import java.io.*;import java.nio.file.*;import java.util.*;
public final class OriginalPrefsTest {
 public static void main(String[] args)throws Exception{
  File app=Files.createTempDirectory("phigros-original-fixture").toFile();File prefs=new File(app,"shared_prefs");prefs.mkdir();File file=new File(prefs,"com.PigeonGames.Phigros.v2.playerprefs.xml");byte[] original="<map><string name='account'>private-fixture</string><int name='score' value='123'/></map>".getBytes("UTF-8");Files.write(file.toPath(),original);
  File root=ShadowStore.prepareOriginal(app);if(!root.equals(app.getCanonicalFile()))throw new AssertionError("redirected");
  File backup=new File(app,"files/phigros-module-backups/original-playerprefs-before-module.xml");if(!Arrays.equals(original,Files.readAllBytes(backup.toPath()))||!Arrays.equals(original,Files.readAllBytes(file.toPath())))throw new AssertionError("original/backup changed");
  Files.write(file.toPath(),"changed".getBytes("UTF-8"));ShadowStore.prepareOriginal(app);if(!Arrays.equals(original,Files.readAllBytes(backup.toPath())))throw new AssertionError("first backup overwritten");
  File absent=Files.createTempDirectory("phigros-absent-fixture").toFile();ShadowStore.prepareOriginal(absent);if(new File(absent,"shared_prefs/com.PigeonGames.Phigros.v2.playerprefs.xml").exists())throw new AssertionError("seeded original without action");
  new File(absent,"shared_prefs").mkdir();Files.write(new File(absent,"shared_prefs/com.PigeonGames.Phigros.v2.playerprefs.xml").toPath(),original);ShadowStore.prepareOriginal(absent);if(!Arrays.equals(original,Files.readAllBytes(new File(absent,"files/phigros-module-backups/original-playerprefs-before-module.xml").toPath())))throw new AssertionError("newly created original not backed up");
  File blocked=Files.createTempDirectory("phigros-blocked-fixture").toFile();new File(blocked,"files").mkdir();Files.write(new File(blocked,"files/phigros-module-backups").toPath(),new byte[]{1});try{ShadowStore.prepareOriginal(blocked);throw new AssertionError("backup failure accepted");}catch(IOException expected){}
  System.out.println("PASS: same original root; exact immutable first backup; no original rewrite or automatic seeding; backup failure rejects activation");
 }
}
