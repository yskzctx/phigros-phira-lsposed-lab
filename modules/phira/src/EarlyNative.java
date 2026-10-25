package com.phira.lab;
/** Subscribe to host-library loads before resolving a class that can load that library. */
public final class EarlyNative {
 public interface Loader {void load()throws Exception;}
 public interface Target<T> {T resolve()throws Exception;}
 public static <T>T loadBeforeTarget(Loader module,Target<T> target)throws Exception{module.load();return target.resolve();}
 /** A findClass-style lazy resolution does not run the host's loadLibrary.
  * Finish its real static initializer before installing a path hook that can
  * attempt activation before the original native entry initializes its owner. */
 public static Class<?> loadAndInitialize(Loader module,String name,ClassLoader loader)throws Exception{
  module.load();return Class.forName(name,true,loader);
 }
 private EarlyNative(){}
}
