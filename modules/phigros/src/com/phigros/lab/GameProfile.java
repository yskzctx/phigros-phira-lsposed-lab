package com.phigros.lab;
public final class GameProfile {
 public static final String TARGET="com.PigeonGames.Phigros",MODULE="com.phigros.lab.lsp";
 public static final String IL2CPP="511e15b9324c171b585ae954e87266437b5de439289918331d1afce9c2b88433";
 public static boolean supports(String pkg,String process,long version,String abi,String nativeHash){return TARGET.equals(pkg)&&TARGET.equals(process)&&version==157&&"arm64-v8a".equals(abi)&&IL2CPP.equals(nativeHash);}
 private GameProfile(){}
}
