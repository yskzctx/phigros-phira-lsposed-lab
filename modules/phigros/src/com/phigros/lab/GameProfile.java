package com.phigros.lab;

/**
 * Which game builds this module attaches to.
 *
 * The native entry picks the matching hook table by comparing each table's prologues
 * against the loaded libil2cpp.so, so this gate only has to confirm the package, the
 * ABI and that the il2cpp image is one we have a table for. Both the 4.0.1 build
 * (157) and the 4.1.0 build (161) are accepted; their hook tables live in
 * core/expected-hooks.h and core/expected-hooks-161.h.
 */
public final class GameProfile {
 public static final String TARGET="com.PigeonGames.Phigros",MODULE="com.phigros.lab.lsp";

 /** Known libil2cpp.so images: version code -> sha256 of the extracted library. */
 public static final String IL2CPP_157="511e15b9324c171b585ae954e87266437b5de439289918331d1afce9c2b88433";
 public static final String IL2CPP_161="b216f4163d5394aae7be3e4b9fbca48fafdca78a19ed3259eab88dff347d1145";

 public static boolean supports(String pkg,String process,long version,String abi,String nativeHash){
  if(!TARGET.equals(pkg)||!TARGET.equals(process)||!"arm64-v8a".equals(abi))return false;
  if(version==157)return IL2CPP_157.equals(nativeHash);
  if(version==161)return IL2CPP_161.equals(nativeHash);
  return false;
 }

 private GameProfile(){}
}
