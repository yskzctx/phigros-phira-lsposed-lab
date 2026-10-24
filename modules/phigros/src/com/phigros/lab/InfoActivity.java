package com.phigros.lab;
import android.app.*;import android.os.Bundle;import android.content.*;import android.widget.*;import com.phigros.offline.ChapterLibraryActivity;
public final class InfoActivity extends Activity {
 @Override public void onCreate(Bundle saved){super.onCreate(saved);LinearLayout box=new LinearLayout(this);box.setOrientation(1);box.setPadding(32,32,32,32);box.setBackgroundColor(0xfff1f5f9);setContentView(box);
  TextView title=new TextView(this);title.setText("Phigros 本地实验模块");title.setTextSize(25);title.setTextColor(0xff17283c);box.addView(title);
  TextView body=new TextView(this);body.setText("开发候选 · 尚未真机验收\n\n只适配 Phigros 4.0.1（157）arm64。请在 LSPosed 中仅勾选官方 Phigros，随后冷启动游戏。音量上打开控制面板，音量下关闭。自动游玩冷启动默认关闭。\n\n测试存档与正版记录分开保存；模拟成绩不上传云存档。禁用模块并冷启动即可回到原始存档。\n\n当前原生章节只替换音符图片。扩展特效、音效和颜色目前用于自制章节。自制章节的文件独立存放在模块中。\n\n安装、作用域加载、原版共存和游玩仍待设备验证。");body.setTextSize(16);body.setTextColor(0xff31445c);body.setPadding(0,24,0,24);box.addView(body);
  Button chapters=new Button(this);chapters.setText("管理自制章节");chapters.setOnClickListener(v->startActivityForResult(new Intent(this,ChapterLibraryActivity.class),1));box.addView(chapters);
  Button skins=new Button(this);skins.setText("自制章节的皮肤、特效与音效");skins.setOnClickListener(v->startActivity(new Intent(this,com.phigros.offline.SkinActivity.class)));box.addView(skins);
 }
}
