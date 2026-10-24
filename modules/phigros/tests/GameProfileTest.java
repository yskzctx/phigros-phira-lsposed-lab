package com.phigros.lab;
public final class GameProfileTest {
 public static void main(String[] args){
  if(!GameProfile.supports("com.PigeonGames.Phigros","com.PigeonGames.Phigros",157,"arm64-v8a",GameProfile.IL2CPP))throw new AssertionError("known profile refused");
  for(String pkg:new String[]{"com.PigeonGames.Phigros.offline","com.PigeonGames.Phigros:remote","org.flos.phira"})if(GameProfile.supports(pkg,pkg,157,"arm64-v8a",GameProfile.IL2CPP))throw new AssertionError("wrong scope");
  if(GameProfile.supports("com.PigeonGames.Phigros","com.PigeonGames.Phigros:remote",157,"arm64-v8a",GameProfile.IL2CPP)||GameProfile.supports("com.PigeonGames.Phigros","com.PigeonGames.Phigros",158,"arm64-v8a",GameProfile.IL2CPP)||GameProfile.supports("com.PigeonGames.Phigros","com.PigeonGames.Phigros",157,"armeabi-v7a",GameProfile.IL2CPP)||GameProfile.supports("com.PigeonGames.Phigros","com.PigeonGames.Phigros",157,"arm64-v8a","wrong"))throw new AssertionError("unverified profile accepted");
  System.out.println("PASS: exact original package/process/version/ABI/native hash only");
 }
}
