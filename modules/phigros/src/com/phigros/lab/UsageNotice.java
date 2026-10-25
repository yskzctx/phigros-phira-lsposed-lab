package com.phigros.lab;
import android.app.*;import android.content.res.ColorStateList;import android.graphics.Typeface;import android.view.*;import android.widget.*;
/** Per-process notice; no network, credentials, or acceptance telemetry. */
public final class UsageNotice {
 public static final String TEXT="本项目为非官方实验工具，与 Phigros、Phira 官方及资源权利人无隶属关系，不代表官方认可或背书。游戏、音乐、谱面、插画与皮肤等权利归各自权利人所有；本项目不额外授予资源的使用或再分发权利。\n\n自动游玩、判定干预、全 AP 与目标 RKS 所产生的数据为模拟数据，不是真实手动游玩成绩，不得据此冒充正常成绩或制作误导性证明。\n\n测试软件可能崩溃、漏判、误判、损坏或改变存档；请先备份，并核对适配版本。仅使用本人有权使用的游戏、谱面和皮肤，不提供购买、账号、服务器、签名完整性或反作弊校验绕过。\n\n模块直接使用原版现有存档，修改前备份。模块不统一断网，保留原版登录和上传流程；是否接受由原版及服务器决定，不承诺上传成功。本地恢复不撤回已上传数据，使用者须遵守平台规则与资源授权。\n\n本说明用于告知权属、功能边界与已知风险，不替代合法授权，不构成免责承诺，也不免除依法应承担的责任。有问题可到对应 GitHub 仓库 Issues 反馈；请勿公开私人存档、账号凭据、签名密钥或设备日志。";
 private static boolean accepted;private static Dialog current;
 public static boolean accepted(){return accepted;}
 public static void show(Activity a,Runnable proceed,Runnable decline){
  if(accepted){proceed.run();return;}if(a.isFinishing()||a.isDestroyed()||current!=null)return;
  ScrollView scroll=new ScrollView(a);TextView content=new TextView(a);content.setText(TEXT);content.setTextSize(15);content.setTextColor(0xff26364a);content.setLineSpacing(5,1);int pad=Math.round(20*a.getResources().getDisplayMetrics().density);content.setPadding(pad,pad,pad,pad);content.setTextIsSelectable(true);scroll.addView(content);
  AlertDialog d=new AlertDialog.Builder(new android.view.ContextThemeWrapper(a,android.R.style.Theme_Material_Light_Dialog_Alert)).setTitle("免责声明与使用范围").setView(scroll).setCancelable(false).setPositiveButton("已阅读，继续",(dialog,which)->{accepted=true;current=null;proceed.run();}).setNegativeButton("暂不使用",(dialog,which)->{current=null;decline.run();}).create();
  d.setOnDismissListener(dialog->{if(current==d)current=null;});current=d;d.show();Window w=d.getWindow();if(w!=null){w.setBackgroundDrawableResource(android.R.color.white);w.setLayout(Math.min(Math.round(620*a.getResources().getDisplayMetrics().density),a.getResources().getDisplayMetrics().widthPixels-24),Math.min(Math.round(580*a.getResources().getDisplayMetrics().density),a.getResources().getDisplayMetrics().heightPixels-24));}
 }
 private UsageNotice(){}
}
