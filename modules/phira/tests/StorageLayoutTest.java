package com.phira.lab;
import java.io.*;import java.nio.charset.StandardCharsets;import java.nio.file.*;import org.json.*;
public final class StorageLayoutTest {
 static void write(File f,String s)throws Exception{Files.createDirectories(f.toPath().getParent());Files.write(f.toPath(),s.getBytes(StandardCharsets.UTF_8));}
 static String read(File f)throws Exception{return new String(Files.readAllBytes(f.toPath()),StandardCharsets.UTF_8);}
 static void check(boolean yes,String why){if(!yes)throw new AssertionError(why);}
 public static void main(String[]args)throws Exception{
  File tmp=Files.createTempDirectory(Paths.get(args[0]),"phira-layout-").toFile(),original=new File(tmp,"original"),priv=new File(tmp,"files");priv.mkdir();
  String actual="{\"tokens\":[\"fixture-token\",\"fixture-refresh\"],\"charts\":[],\"local_records\":{\"original\":null}}";
  write(new File(original,"data/data.json"),actual);write(new File(original,"data.json"),"wrong parent sentinel");
  File lab=LabStore.prepare(original,priv),engine=new File(lab,"data/data.json");
  check(engine.isFile(),"module wrote data.json above the original engine's data/ root");
  check(read(new File(lab,"original-game-data-backup-v2.json")).equals(actual),"backed up wrong original file");
  check(read(new File(original,"data/data.json")).equals(actual)&&read(new File(original,"data.json")).equals("wrong parent sentinel"),"original was changed");
  JSONObject fresh=new JSONObject(read(engine));check(fresh.isNull("tokens")&&fresh.isNull("me"),"original account copied");
  File priv2=new File(tmp,"files-old");priv2.mkdir();File old=new File(priv2,"phira-lab-082");old.mkdir();
  write(new File(old,"initialized-v1"),"1");write(new File(old,"data.json"),"{\"charts\":[],\"local_records\":{}}");
  String played="{\"charts\":[{\"local_path\":\"custom/real\",\"record\":{\"score\":800000}}],\"respacks\":[\"kept\"],\"local_records\":{},\"config\":{}}";
  write(new File(old,"data/data.json"),played);write(new File(old,"session-records.json"),"{\"version\":2,\"local_records\":{},\"chart_records\":{}}");write(new File(old,"history-pending.json"),"{\"operation\":\"all-ap\"}");
  LabStore.prepare(original,priv2);JSONObject upgraded=new JSONObject(read(new File(old,"data/data.json")));
  check(upgraded.getJSONArray("charts").getJSONObject(0).getJSONObject("record").getInt("score")==800000,"legacy wrong-root pending operation overwrote real engine history");
  check(upgraded.getJSONArray("respacks").getString(0).equals("kept"),"engine skin index lost");
  check(new File(old,"legacy-layout-v1/session-records.json").isFile()&&new File(old,"legacy-layout-v1/history-pending.json").isFile(),"legacy metadata not retained");
  check(!new File(old,"session-records.json").exists(),"wrong-root session pending survived migration");
  LabStore.queueHistory(old,"all-ap");LabStore.prepare(original,priv2);check(new JSONObject(read(new File(old,"data/data.json"))).getJSONArray("charts").getJSONObject(0).getJSONObject("record").getInt("score")==1000000,"AP did not reach actual engine file");
  LabStore.queueHistory(old,"restore");LabStore.prepare(original,priv2);check(new JSONObject(read(new File(old,"data/data.json"))).getJSONArray("charts").getJSONObject(0).getJSONObject("record").getInt("score")==800000,"restore did not reach actual engine file");
  System.out.println("PASS: actual engine data/ layout, correct original backup, v1 metadata quarantine, imported resources and AP restore retained");
 }
}
