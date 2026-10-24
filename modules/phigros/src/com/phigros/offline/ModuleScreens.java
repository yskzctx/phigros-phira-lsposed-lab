package com.phigros.offline;
import android.app.*;import android.content.*;import android.view.ContextThemeWrapper;import android.widget.*;import java.io.*;import java.util.*;
import com.phigros.lab.*;
/** Host-owned document grants; imported files stay in the independent lab. */
public final class ModuleScreens {
 private static final int PICK_SKIN=0x6d02;
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
  if(request!=PICK_SKIN||pending.remove(activity)==null)return false;
  if(result!=Activity.RESULT_OK||data==null||data.getData()==null)return true;
  new Thread(()->{String outcome;try(InputStream in=activity.getContentResolver().openInputStream(data.getData())){if(in==null)throw new IOException("无法读取图包");SkinPack pack=SkinController.importPack(LabContext.wrap(activity),in);SkinController.select(LabContext.wrap(activity),pack.id);outcome="已导入皮肤，冷启动后应用";}catch(Exception error){outcome=String.valueOf(error.getMessage());}final String text=outcome;activity.runOnUiThread(()->message(activity,text));},"LabSkinImport").start();return true;
 }
 private static void message(Activity a,String text){Toast.makeText(a,text,Toast.LENGTH_LONG).show();}
 public static void chapters(Activity activity){
  ControlsConfig c=NativeControls.load(LabContext.wrap(activity));Intent i=new Intent().setComponent(new ComponentName(GameProfile.MODULE,ChapterLibraryActivity.class.getName()));
  i.putExtra("lab.mode",c.mode).putExtra("lab.score",Integer.toString(c.target)).putExtra("lab.accuracy",new java.math.BigDecimal(c.accuracy).divide(new java.math.BigDecimal(100)).toPlainString()).putExtra("lab.rules",c.encodedRules).putExtra("lab.autoplay",LaunchActivity.isAutoplay());activity.startActivityForResult(i,0x6d01);
 }
 private ModuleScreens(){}
}
