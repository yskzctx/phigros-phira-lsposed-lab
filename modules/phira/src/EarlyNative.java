package com.phira.lab;
/** Subscribe to host-library loads before resolving a class that can load that library. */
public final class EarlyNative {
 public interface Loader {void load()throws Exception;}
 public interface Target<T> {T resolve()throws Exception;}
 public static <T>T loadBeforeTarget(Loader module,Target<T> target)throws Exception{module.load();return target.resolve();}
 private EarlyNative(){}
}
