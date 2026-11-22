package com.phigros.offline;
import android.app.*;import android.content.*;import android.view.ContextThemeWrapper;import android.widget.*;import java.io.*;import java.util.*;
import com.phigros.lab.*;
/** Host-owned document grants; imported files stay in the independent lab. */
public final class ModuleScreens {
 private static final int PICK_SKIN=0x6d02;
 private static final int PICK_SAVE=0x6d03;
 private static final WeakHashMap<Activity,Boolean> pending=new WeakHashMap<>();
 public static void skin(Activity activity){
  Context context=LabContext.wrap(activity);try{
   List<SkinPack> packs=SkinController.list(context);ArrayList<String> names=new ArrayList<>();names.add("导入 Phira 皮肤 ZIP");names.add("恢复默认皮肤");for(SkinPack p:packs)names.add(p.name+" · "+p.author);
   new AlertDialog.Builder(new ContextThemeWrapper(activity,android.R.style.Theme_Material_Light_Dialog_Alert)).setTitle("音符皮肤 · 选择后冷启动应用").setItems(names.toArray(new String[0]),(d,index)->{
    try{if(index==0){pending.put(activity,true);Intent i=new Intent(Intent.ACTION_OPEN_DOCUMENT).addCategory(Intent.CATEGORY_OPENABLE).setType("*/*");activity.startActivityForResult(i,PICK_SKIN);}
    else{SkinController.select(context,index==1?"":packs.get(index-2).id);Toast.makeText(activity,"皮肤已选择，冷启动后应用",Toast.LENGTH_LONG).show();}}
    catch(Exception error){message(activity,error.getMessage());}
   }).setNegativeButton("关闭",null).show();
  }catch(IOException error){message(activity,error.getMessage());}
 }
 public static boolean result(Activity activity,int request,int result,Intent data){
  if(request==PICK_SAVE)return saveResult(activity,request,result,data);
  if(request!=PICK_SKIN||pending.remove(activity)==null)return false;
  if(result!=Activity.RESULT_OK||data==null||data.getData()==null)return true;
  new Thread(()->{String outcome;try(InputStream in=activity.getContentResolver().openInputStream(data.getData())){if(in==null)throw new IOException("无法读取图包");SkinPack pack=SkinController.importPack(LabContext.wrap(activity),in);SkinController.select(LabContext.wrap(activity),pack.id);outcome="已导入皮肤，冷启动后应用";}catch(Exception error){outcome=String.valueOf(error.getMessage());}final String text=outcome;activity.runOnUiThread(()->message(activity,text));},"LabSkinImport").start();return true;
 }
 private static void message(Activity a,String text){Toast.makeText(a,text,Toast.LENGTH_LONG).show();}
 /**
  * Save import / export from the module.
  *
  * The module runs inside the unmodified original game, so the save it edits is the real
  * one. The flow is deliberately the same as the standalone build: inspect and summarise
  * the picked file, confirm, back up the current save, then write - and tell the user to
  * restart, because the game caches preferences in memory.
  */
 public static void saveTransfer(Activity activity){
  final Context context=LabContext.wrap(activity);
  String[] items={"导入存档文件","导出当前存档","恢复备份"};
  new AlertDialog.Builder(new ContextThemeWrapper(activity,android.R.style.Theme_Material_Light_Dialog_Alert))
   .setTitle("导入 / 导出存档")
   .setItems(items,(d,index)->{
    try{
     if(index==0){pending.put(activity,true);Intent i=new Intent(Intent.ACTION_OPEN_DOCUMENT).addCategory(Intent.CATEGORY_OPENABLE).setType("*/*");activity.startActivityForResult(i,PICK_SAVE);}
     else if(index==1){pending.put(activity,true);Intent i=new Intent(Intent.ACTION_CREATE_DOCUMENT).addCategory(Intent.CATEGORY_OPENABLE).setType("application/zip");i.putExtra(Intent.EXTRA_TITLE,"phigros-save.zip");activity.startActivityForResult(i,PICK_SAVE+100);}
     else{restoreMenu(activity,context);}
    }catch(Exception error){message(activity,error.getMessage());}
   }).setNegativeButton("关闭",null).show();
 }
 private static void restoreMenu(Activity activity,final Context context){
  List<File> list=SaveTransfer.backups(context);
  if(list.isEmpty()){message(activity,"还没有备份");return;}
  int count=Math.min(8,list.size());
  String[] labels=new String[count];
  for(int i=0;i<count;i++)labels[i]=SaveTransfer.label(list.get(i));
  new AlertDialog.Builder(new ContextThemeWrapper(activity,android.R.style.Theme_Material_Light_Dialog_Alert))
   .setTitle("恢复备份（当前存档会先备份）")
   .setItems(labels,(d,index)->{
    try{SaveTransfer.restore(context,list.get(index));message(activity,"已恢复，请完全退出并重启游戏");}
    catch(IOException error){message(activity,String.valueOf(error.getMessage()));}
   }).setNegativeButton("关闭",null).show();
 }
 private static boolean saveResult(final Activity activity,int request,int result,Intent data){
  boolean exporting=request==PICK_SAVE+100;
  if(pending.remove(activity)==null)return false;
  if(result!=Activity.RESULT_OK||data==null||data.getData()==null)return true;
  final android.net.Uri uri=data.getData();
  final Context context=LabContext.wrap(activity);
  new Thread(()->{
   String outcome;
   try{
    if(exporting){
     try(OutputStream out=activity.getContentResolver().openOutputStream(uri)){
      if(out==null)throw new IOException("无法写入所选位置");
      SaveTransfer.export(context,out);
     }
     outcome="存档已导出";
    }else{
     SaveTransfer.Plan plan;
     try(InputStream in=activity.getContentResolver().openInputStream(uri)){
      if(in==null)throw new IOException("无法读取所选文件");
      plan=SaveTransfer.inspect(in);
     }
     // Confirm before writing: the summary is the only chance to notice a wrong file.
     final SaveTransfer.Plan inspected=plan;
     activity.runOnUiThread(()->new AlertDialog.Builder(new ContextThemeWrapper(activity,android.R.style.Theme_Material_Light_Dialog_Alert))
      .setTitle("导入这个存档？")
      .setMessage(inspected.describe()+"\n\n当前存档会先备份，可随时恢复。导入后需完全退出并重启游戏。")
      .setPositiveButton("导入",(d,w)->{
       try{SaveTransfer.apply(context,inspected);message(activity,"已导入，请完全退出并重启游戏");}
       catch(IOException error){message(activity,String.valueOf(error.getMessage()));}
      }).setNegativeButton("取消",null).show());
     return;
    }
   }catch(Exception error){outcome=String.valueOf(error.getMessage());}
   final String text=outcome;
   activity.runOnUiThread(()->message(activity,text));
  },"LabSaveTransfer").start();
  return true;
 }
 public static void chapters(Activity activity){
  ControlsConfig c=NativeControls.load(LabContext.wrap(activity));Intent i=new Intent().setComponent(new ComponentName(GameProfile.MODULE,ChapterLibraryActivity.class.getName()));
  i.putExtra("lab.mode",c.mode).putExtra("lab.score",Integer.toString(c.target)).putExtra("lab.accuracy",new java.math.BigDecimal(c.accuracy).divide(new java.math.BigDecimal(100)).toPlainString()).putExtra("lab.rules",c.encodedRules).putExtra("lab.autoplay",LaunchActivity.isAutoplay());activity.startActivityForResult(i,0x6d01);
 }
 private ModuleScreens(){}
}
