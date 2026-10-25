package com.phira.lab;
public final class DiagnosticStateTest {
 public static void main(String[]args){
  check(DiagnosticState.describe(false,new int[]{0,0,0,0,0,0,0},null),"原生回调未注册");
  check(DiagnosticState.describe(false,new int[]{0,0,0,0,0,1,0},null),"等待游戏原生库");
  check(DiagnosticState.describe(false,new int[]{0,0,0,0,0,3,0},null),"函数特征不匹配");
  check(DiagnosticState.describe(false,new int[]{1,0,0,0,0,5,1},null),"游戏已提前运行");
  check(DiagnosticState.describe(false,new int[]{1,1,0,0,0,5,0},null),"独立存档未切换");
  check(DiagnosticState.describe(false,new int[]{1,0,0,0,0,5,0},null),"原生 Hook 已安装，独立存档尚未激活");
  check(DiagnosticState.describe(false,new int[]{0,0,0,0,0,5,0},null),"原生就绪状态异常");
  check(DiagnosticState.describe(true,new int[]{1,1,0,0,0,5,0},null),"已接入 · 自动游玩关闭");
  check(DiagnosticState.describe(true,new int[]{1,1,1,1,12,5,0},null),"已接入 · 自动游玩开启");
  check(DiagnosticState.describe(true,new int[]{1,1},null),"原生状态格式异常");
  check(DiagnosticState.describe(true,null,"版本不匹配"),"版本不匹配");
  String report=DiagnosticState.report(false,new int[]{1,0,0,0,0,5,0},null,"等待游戏启动",0,0);
  if(!report.contains("N=1 A=0 S=5 E=0 R=0 P=0 T=0"))throw new AssertionError(report);
  if(report.startsWith("已接入"))throw new AssertionError("unarmed reported connected");
  System.out.println("Diagnostic state tests passed: failures never appear connected.");
 }
 static void check(String actual,String expected){if(!expected.equals(actual))throw new AssertionError(actual+" != "+expected);}
}
