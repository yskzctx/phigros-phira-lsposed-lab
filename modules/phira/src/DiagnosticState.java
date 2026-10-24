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
  if(s[0]!=1||s[1]!=1)return "原生接口未就绪";
  if(!redirected)return "独立存档未切换";
  return s[2]==1?"已接入 · 自动游玩开启":"已接入 · 自动游玩关闭";
 }
 private DiagnosticState(){}
}
