import android.content.*;import com.phigros.offline.*;
public class BootstrapGuardTest {
 public static void main(String[] a)throws Exception{
  Context original=new Context(){public String getPackageName(){return "com.PigeonGames.Phigros";}};
  original.getSharedPreferences("offline_controls",0).edit().putBoolean("data_10000mb_v165",true).commit();
  RksEditor.applyPending(original);
  String[] lines=new String(java.nio.file.Files.readAllBytes(java.nio.file.Paths.get(System.getProperty("fixtureAssets"),"offline/chart-catalog.tsv")),"UTF-8").split("\\n");
  java.lang.reflect.Method encrypt=RksEditor.class.getDeclaredMethod("encrypt",String.class);encrypt.setAccessible(true);
  String key=(String)encrypt.invoke(null,lines[0].split("\\t")[0]),other=(String)encrypt.invoke(null,lines[1].split("\\t")[0]);
  java.io.File backup=new java.io.File(System.getProperty("fixture"),"backup.xml"),current=new java.io.File(System.getProperty("fixture"),"current.xml");
  java.nio.file.Files.write(backup.toPath(),("<map><string name=\""+key+"\">old-record</string><string name=\"account\">old-account</string></map>").getBytes("UTF-8"));
  java.nio.file.Files.write(current.toPath(),("<map><string name=\""+key+"\">auto-record</string><string name=\""+other+"\">new-auto-record</string><string name=\"account\">updated-account</string><int name=\"setting\" value=\"9\"/></map>").getBytes("UTF-8"));
  RksEditor.restoreSessionRecords(original,backup,current);String restored=new String(java.nio.file.Files.readAllBytes(current.toPath()),"UTF-8");
  if(!restored.contains("old-record")||restored.contains("auto-record")||!restored.contains("updated-account")||!restored.contains("value=\"9\""))throw new AssertionError("record-only restore corrupted unrelated state");
  java.lang.reflect.Method replace=RksEditor.class.getDeclaredMethod("replace",String.class,java.util.Map.class);replace.setAccessible(true);java.util.Map<String,String> records=new java.util.HashMap<>();records.put(key,"new-history");String inserted=(String)replace.invoke(null,"<map><string name=\"account\">preserved</string></map>",records);if(!inserted.contains("new-history")||!inserted.contains("preserved"))throw new AssertionError("missing original chart record not inserted");
  try{RksEditor.applyPending(new Context(){public String getPackageName(){return "unrelated.app";}});throw new AssertionError("wrong host accepted");}catch(java.io.IOException expected){}
  System.out.println("PASS: real RksEditor accepts official host; rejects unrelated host; session restores only chart records, preserving updated account/settings");
 }
}
