package com.phira.lab;
import java.math.BigDecimal;import java.math.RoundingMode;
/** Decimal milliseconds, stored as tenths; independent of audio calibration. */
public final class TimingSetting {
 public static int parse(String text){if(text==null||!text.trim().matches("[0-9]+(\\.[0-9])?"))throw new IllegalArgumentException("结算误差请填写0～75毫秒，最多一位小数；0为关闭");try{int value=new BigDecimal(text.trim()).movePointRight(1).setScale(0,RoundingMode.UNNECESSARY).intValueExact();if(value<0||value>750)throw new ArithmeticException();return value;}catch(ArithmeticException e){throw new IllegalArgumentException("结算误差范围为0～75毫秒");}}
 public static String format(int value){return BigDecimal.valueOf(value,1).stripTrailingZeros().toPlainString();}
 private TimingSetting(){}
}
