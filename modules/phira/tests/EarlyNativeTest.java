package com.phira.lab;
import java.util.ArrayList;
public final class EarlyNativeTest {
 static final ArrayList<String> events=new ArrayList<>();static boolean registered;static int received;
 static final class GameA {static {events.add("game-library");if(registered)received++;}}
 static final class GameB {static {events.add("game-library");if(registered)received++;}}
 static void register(){events.add("module-callback");registered=true;}
 public static void main(String[]args)throws Exception{
  // Reproduce a missed library callback when resolving initializes the host class first.
  Class.forName(GameA.class.getName(),true,EarlyNativeTest.class.getClassLoader());register();
  if(received!=0)throw new AssertionError("negative baseline did not miss the callback");
  registered=false;events.clear();
  EarlyNative.loadBeforeTarget(EarlyNativeTest::register,()->Class.forName(GameB.class.getName(),true,EarlyNativeTest.class.getClassLoader()));
  if(received!=1||!events.toString().equals("[module-callback, game-library]"))throw new AssertionError(events.toString());
  boolean[] resolved={false};try{EarlyNative.loadBeforeTarget(()->{throw new UnsatisfiedLinkError("fixture");},()->{resolved[0]=true;return GameB.class;});throw new AssertionError("failure swallowed");}catch(UnsatisfiedLinkError expected){}
  if(resolved[0])throw new AssertionError("target initialized after module loader failure");
  System.out.println("PASS: actual JVM static initialization callback order, negative baseline, fail-closed loader");
 }
}
