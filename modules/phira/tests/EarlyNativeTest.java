package com.phira.lab;
import java.util.ArrayList;
public final class EarlyNativeTest {
 static final ArrayList<String> events=new ArrayList<>();static boolean registered;static int received;
 static final class GameA {static {events.add("game-library");if(registered)received++;}}
 static final class GameB {static {events.add("game-library");if(registered)received++;}}
 static final class LazyGame {static {events.add("game-library");if(registered)received++;}}
 static final class InitializedGame {static {events.add("game-library");if(registered)received++;}}
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
  registered=false;received=0;events.clear();
  // Match findClass's actual initialize=false behavior, rather than the old
  // positive test's initialize=true resolver. Activating before the original
  // static entry initializes its owner loses the only activation attempt.
  Class<?> lazy=EarlyNative.loadBeforeTarget(EarlyNativeTest::register,()->Class.forName(LazyGame.class.getName(),false,EarlyNativeTest.class.getClassLoader()));
  boolean armed=received==1;if(armed)throw new AssertionError("lazy resolution unexpectedly initialized native code");
  Class.forName(lazy.getName(),true,lazy.getClassLoader());if(received!=1||armed)throw new AssertionError("negative late-ready baseline");
  registered=false;received=0;events.clear();
  EarlyNative.loadAndInitialize(EarlyNativeTest::register,InitializedGame.class.getName(),EarlyNativeTest.class.getClassLoader());
  armed=received==1;if(!armed||!events.toString().equals("[module-callback, game-library]"))throw new AssertionError("activation attempted before native initialization: "+events);
  System.out.println("PASS: actual JVM initialization order; lazy-find baseline reproduces late-ready/unarmed state; eager initialization and fail-closed loader");
 }
}
