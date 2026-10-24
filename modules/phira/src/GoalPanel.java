package com.phira.lab;
import android.app.*;import android.content.*;import android.content.res.AssetManager;import android.os.*;import android.view.*;import android.webkit.*;import android.widget.*;import java.io.*;import java.nio.charset.StandardCharsets;import java.math.BigDecimal;import org.json.*;
/** Shared, in-panel rule picker. Its WebView cannot make network requests; the
 * host's login/network/eligibility logic remains outside this local editor. */
public final class GoalPanel {
 private final Activity activity;private final Handler handler=new Handler(Looper.getMainLooper());private final WebView web;private boolean destroyed;
 private final Runnable ticker=new Runnable(){public void run(){if(!destroyed){publish(false,null);handler.postDelayed(this,1200);}}};
 public GoalPanel(Activity a){activity=a;web=new WebView(a);WebSettings s=web.getSettings();s.setJavaScriptEnabled(true);s.setAllowFileAccess(false);s.setAllowContentAccess(false);s.setBlockNetworkLoads(true);s.setMixedContentMode(WebSettings.MIXED_CONTENT_NEVER_ALLOW);s.setSaveFormData(false);s.setDomStorageEnabled(false);web.setBackgroundColor(0xfff1f5f9);web.addJavascriptInterface(new Bridge(),"OfflineBridge");web.setWebViewClient(new WebViewClient(){@Override public void onPageFinished(WebView v,String url){publish(true,null);}});
  try{AssetManager assets;try{assets=a.createPackageContext(ConnectionReport.MODULE,0).getAssets();}catch(android.content.pm.PackageManager.NameNotFoundException unavailable){if(Entry.modulePath==null)throw new IOException("模块资源路径未提供");assets=android.content.res.XModuleResources.createInstance(Entry.modulePath,null).getAssets();}
   ByteArrayOutputStream bytes=new ByteArrayOutputStream();try(InputStream in=assets.open("controls-ui.html")){byte[] b=new byte[8192];int n;while((n=in.read(b))!=-1){if(bytes.size()+n>512*1024)throw new IOException("模块界面过大");bytes.write(b,0,n);}}
   web.loadDataWithBaseURL("https://phira-module.invalid/",new String(bytes.toByteArray(),StandardCharsets.UTF_8),"text/html","UTF-8",null);handler.postDelayed(ticker,1200);
  }catch(Exception e){web.loadData("<html><body style='background:#f1f5f9;color:#17283c;font:18px sans-serif;padding:20px'><h3>模块界面资源未就绪</h3><p>请核对当前模块安装是否完整。音量下键可收起；模块入口页可以查看启动诊断。</p></body></html>","text/html","UTF-8");}
 }
 public View view(){return web;}
 public void destroy(){destroyed=true;handler.removeCallbacksAndMessages(null);web.removeJavascriptInterface("OfflineBridge");web.destroy();}
 private final class Bridge {
  @JavascriptInterface public String environment(){return "phira-module";}
  @JavascriptInterface public void submit(String action,String payload){if(action==null||payload==null||payload.length()>100000)return;handler.post(()->handle(action,payload));}
 }
 private void handle(String action,String payload){if(destroyed)return;try{
  JSONObject p=new JSONObject(payload);boolean reload=false;
  if(action.equals("ready")){GoalControls.apply(GoalControls.load(activity));reload=true;}
  else if(action.equals("save")){ControlsConfig c=ControlsConfig.parse(p.getInt("mode"),p.optString("score",""),p.optString("accuracy",""),p.optString("rules",""));GoalControls.save(activity,c);publish(true,"已保存，下次开始谱面时使用新计划");return;}
  else if(action.equals("reset")){GoalControls.save(activity,ControlsConfig.parse(0,"1000000","100",""));publish(true,"已重置目标与规则，下局生效");return;}
  else if(action.equals("auto")){
   int[] s=LabNative.safeStatus();if(!Entry.redirected||Entry.root==null||s[0]!=1||s[1]!=1)throw new IllegalStateException(Entry.connection());boolean on=p.getBoolean("enabled");
   if(on)LabStore.beginAutoplay(Entry.root);GoalControls.apply(GoalControls.load(activity));if(!LabNative.setEnabled(on))throw new IllegalStateException("原生接口未就绪");
  }else if(action.equals("all-ap")||action.equals("restore")){
   if(!Entry.redirected||Entry.root==null)throw new IllegalStateException("独立实验存档未接入");LabStore.queueHistory(Entry.root,action.equals("all-ap")?"all-ap":"restore");publish(false,"历史操作已保存，完全退出并冷启动后生效");return;
  }else throw new IllegalArgumentException("当前模块不支持此操作");
  publish(reload,null);
 }catch(Throwable e){publish(false,e.getMessage()==null?e.getClass().getSimpleName():e.getMessage());}}
 private void publish(boolean reload,String message){if(destroyed)return;try{
  ControlsConfig c=GoalControls.load(activity);int[] s=GoalControls.status();JSONObject data=new JSONObject();data.put("auto",s[1]==1).put("mode",c.mode).put("score",Integer.toString(c.target)).put("accuracy",new BigDecimal(c.accuracy).divide(new BigDecimal(100)).toPlainString()).put("reload",reload).put("rksTarget","").put("skin","由 Phira 原版管理");
  JSONArray rules=new JSONArray();for(ControlsConfig.Rule r:c.rules){StringBuilder list=new StringBuilder();for(int id:r.indices){if(list.length()>0)list.append(',');list.append(id);}rules.put(new JSONObject().put("judgment",r.judgment).put("position",r.position).put("start",r.start).put("end",r.end).put("count",r.count).put("indices",list.toString()).put("noteType",r.noteType));}data.put("rules",rules);
  String runtime=Entry.connection();String detail="";try{detail=LabNative.controlDetail();}catch(UnsatisfiedLinkError ignored){}
  if(s[0]==1&&s[1]==1&&s[2]>0){runtime="音符 "+s[2]+" · 计划 "+s[6]+" 分 / "+new BigDecimal(s[7]).divide(new BigDecimal(100)).toPlainString()+"% · P "+(s[2]-s[8]-s[9]-s[10])+" G "+s[9]+" B "+s[10]+" M "+s[8];runtime+="\n"+(s[11]==1?"完整判定事件统计":"模块启用期间事件统计")+"：P "+s[14]+" G "+s[15]+" B "+s[16]+" M "+s[17]+" · 最大连击 "+s[18]+" · Good早/晚 "+s[19]+"/"+s[20]+" · 推算 "+s[12]+" 分。最终以原版结算为准。";}
  if(detail!=null&&!detail.isEmpty())runtime+="\n"+detail;
  data.put("runtime",runtime).put("history","历史修改在冷启动后生效，原始存档保留。自动会话的本地恢复不撤回已上传记录。");if(message!=null)data.put("message",message).put("saved",reload);
  web.evaluateJavascript("window.receive&&window.receive("+data.toString()+");",null);
 }catch(Exception ignored){}}
}
