package com.phira.lab;
public final class ControlsConfigTest {
 private static void check(boolean yes,String message){if(!yes)throw new AssertionError(message);}
 private static void reject(int mode,String score,String acc,String rules){try{ControlsConfig.parse(mode,score,acc,rules);}catch(IllegalArgumentException expected){return;}throw new AssertionError("invalid configuration accepted");}
 public static void main(String[] args){
  ControlsConfig accuracy=ControlsConfig.parse(2,"","90.25","");
  check(accuracy.accuracy==9025&&accuracy.target==1000000,"hidden empty score blocked accuracy");
  ControlsConfig score=ControlsConfig.parse(1,"900000","","");check(score.target==900000&&score.accuracy==10000,"hidden accuracy blocked score");
  ControlsConfig all=ControlsConfig.parse(0,"","","");check(all.rules.length==0,"default AP");
  ControlsConfig typed=ControlsConfig.parse(0,"","","4:3:1:1:1:10,25,10:3;5:2:50:80:1::4;0:6:1:1:1::0");
  check(typed.packed[0]==-168&&typed.packed[1]==3,"typed header");check(typed.rules[0].indices.length==2&&typed.rules[0].judgment==4&&typed.rules[0].noteType==3,"early Hold/list dedup");check(typed.rules[1].judgment==5&&typed.rules[2].judgment==0,"late and Perfect");
  for(int j=0;j<=5;j++)for(int p=1;p<=6;p++)for(int t=0;t<=4;t++)ControlsConfig.parse(0,"","",j+":"+p+":1:3:2:"+(p==3?"1,3":"")+":"+t);
  reject(1,"","90","");reject(2,"900000","","");reject(2,"","100.01","");reject(2,"","NaN","");reject(1,"1000001","","");reject(0,"","","4:2:80:50:1::0");reject(0,"","","4:3:1:1:1::0");reject(0,"","","6:1:1:1:1::0");reject(0,"","","4:1:1:1:1::5");
  StringBuilder rows=new StringBuilder();for(int i=0;i<17;i++){if(i>0)rows.append(';');rows.append("0:6:1:1:1::0");}reject(0,"","",rows.toString());
  System.out.println("PASS: active targets, all rule judgments/positions/types, typed JNI schema and invalid boundaries");
 }
}
