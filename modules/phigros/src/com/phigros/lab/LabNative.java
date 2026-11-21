package com.phigros.lab;
public final class LabNative {
 public static native boolean setRoot(String path);
 public static native boolean isReady();
 public static native boolean readyOrAbort();
 public static native int[] diagnostics();
 public static boolean ready(){try{return isReady();}catch(UnsatisfiedLinkError error){return false;}}
 /** 云同步观察器自检：{已挂载, 隔离开关, 同步调用次数, 静默调用次数, 最近一次同步结果}。 */
 public static native int[] cloudDiagnostics();
 /** 把云同步自检翻译成面板能直接显示的一句话，避免用户只看到“同步失败”。 */
 public static String cloudSummary(){
  try{
   int[] d=cloudDiagnostics();
   if(d==null||d.length<5)return "云同步自检不可用";
   if(d[0]==0)return "云同步观察器未挂载：宿主版本或原生库不匹配，模块未改动原版同步";
   if(d[2]==0&&d[3]==0)return "观察器已挂载，但本次启动宿主尚未调用同步接口（可能还没到结算或未登录）";
   return "观察器已挂载：同步调用 "+d[2]+" 次 / 静默调用 "+d[3]+" 次，最近结果 "+(d[4]!=0?"成功":"未完成")
     +"；模块只观察不修改，上传结果仍由原版与服务器决定";
  }catch(UnsatisfiedLinkError error){return "云同步自检不可用：原生库未加载";}
 }
 private LabNative(){}
}
