package com.phigros.lab;
import android.content.Intent;import android.widget.LinearLayout;
public final class InfoActivity extends Dashboard {
 protected void extraCards(LinearLayout page){LinearLayout c=card(page);heading(c,"自制章节");body(c,"模块自己的独立资源库。管理谱面与皮肤，不修改原版资源目录。");action(c,"管理自制谱",false,()->startActivityForResult(new Intent(this,com.phigros.offline.ChapterLibraryActivity.class),0x6d01));action(c,"皮肤、特效与音效",false,()->startActivity(new Intent(this,com.phigros.offline.SkinActivity.class)));}
}
