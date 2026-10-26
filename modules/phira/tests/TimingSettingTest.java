package com.phira.lab;
public class TimingSettingTest {public static void main(String[] a){
 if(TimingSetting.parse("20.5")!=205||TimingSetting.parse("0")!=0||TimingSetting.parse("75")!=750||!TimingSetting.format(205).equals("20.5"))throw new AssertionError();
 for(String s:new String[]{"","-1","75.1","NaN","1e1","0.12"}){try{TimingSetting.parse(s);throw new AssertionError(s);}catch(IllegalArgumentException expected){}}
 System.out.println("PASS: decimal milliseconds, zero disables, bounds, non-finite/exponent/precision rejected");
}}
