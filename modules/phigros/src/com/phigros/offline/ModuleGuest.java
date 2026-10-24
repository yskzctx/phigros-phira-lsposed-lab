package com.phigros.offline;
import android.app.Activity;import android.widget.Toast;import com.phigros.lab.GameProfile;
public final class ModuleGuest {
 public static boolean prepare(Activity activity){
  String caller=activity.getCallingPackage();if(!GameProfile.TARGET.equals(caller)&&!GameProfile.MODULE.equals(caller)){Toast.makeText(activity,"请从模块或游戏控制面板打开",Toast.LENGTH_LONG).show();activity.finish();return false;}
  try{android.content.Intent i=activity.getIntent();if(i.hasExtra("lab.mode")){ControlsConfig c=ControlsConfig.parse(i.getIntExtra("lab.mode",0),i.getStringExtra("lab.score"),i.getStringExtra("lab.accuracy"),i.getStringExtra("lab.rules"));NativeControls.save(activity,c);}
   LaunchActivity.setGuestAutoplay(i.getBooleanExtra("lab.autoplay",false));return true;
  }catch(Exception error){Toast.makeText(activity,"章节配置无法读取",Toast.LENGTH_LONG).show();activity.finish();return false;}
 }
 private ModuleGuest(){}
}
