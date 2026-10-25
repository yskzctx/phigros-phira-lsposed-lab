package com.phira.lab;
/** Pure state decoder shared by host diagnostics and the control panel. */
public final class DiagnosticState {
 public static String describe(boolean redirected,int[] s,String failure){
  if(failure!=null&&!failure.isEmpty())return failure;
  if(s==null||s.length<7)return "原生状态格式异常";
  if(s[6]!=0)return "游戏已提前运行";
  switch(s[5]){
   case -1:return "模块原生库未加载";
   case 0:return "原生回调未注册";
   case 1:return "等待游戏原生库";
   case 2:return "游戏原生符号无法定位";
   case 3:return "函数特征不匹配";
   case 4:return "原生 Hook 安装失败";
   case 5:break;
   default:return "原生状态格式异常";
  }
  if(s[0]!=1)return "原生就绪状态异常";
  if(s[1]!=1)return "原生 Hook 已安装，独立存档尚未激活";
  if(!redirected)return "独立存档未切换";
  return s[2]==1?"已接入 · 自动游玩开启":"已接入 · 自动游玩关闭";
 }
 public static String report(boolean redirected,int[] s,String failure,String phase,int pathCalls,int armAttempts){
  String text=describe(redirected,s,failure);
  if(s==null||s.length<7)return text;
  return text+"\n阶段："+phase+"\n诊断 N="+s[0]+" A="+s[1]+" S="+s[5]+" E="+s[6]+" R="+(redirected?1:0)+" P="+pathCalls+" T="+armAttempts;
 }
 private DiagnosticState(){}
}
