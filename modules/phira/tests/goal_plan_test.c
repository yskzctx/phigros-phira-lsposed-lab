#include <assert.h>
#include <math.h>
#include <stdio.h>
#include "../native/goal-plan.h"
int main(void){
 unsigned char p[11]={0};p[9]=2;p[10]=5;PlanEstimate e;estimate_plan(p,10,&e);assert(e.score==937000&&e.accuracy==9300&&e.good==2&&e.max_combo==10);
 p[10]=1;estimate_plan(p,10,&e);assert(e.score==868500&&e.accuracy==8650&&e.max_combo==9);
 PlanTiming times[4]={{0},{1,10},{2,2},{3,3}};unsigned char types[4]={0,3,1,1};PlanTimeline t=plan_timeline_windows(times,types,3,.12f,.221f,.19f,123);unsigned char ranked[4]={0,0,1,0};estimate_timed(ranked,3,&e,&t);assert(e.max_combo==2&&e.score==666667);free(t.events);
 ControlConfig c={.mode=2,.accuracy=9000,.rule_count=1};c.rules[0]=(NoteRule){.judgment=0,.position=1,.start=1};unsigned char out[101],kinds[101];memset(kinds,1,sizeof(kinds));make_profile_typed(&c,100,123,out,&e,kinds);assert(out[1]==0&&abs(e.accuracy-9000)<=35);
 c=(ControlConfig){.rule_count=1};c.rules[0]=(NoteRule){.judgment=4,.position=6,.note_type=4};memset(kinds,1,sizeof(kinds));kinds[2]=4;make_profile_typed(&c,100,123,out,&e,kinds);assert(out[2]==4&&out[1]==0&&e.good==1);
 PlanTiming one[2]={{0},{1,1}};unsigned char tap[2]={0,1};t=plan_timeline_windows(one,tap,1,.12f,.221f,.19f,123);int early=0,late=0;
 for(int k=0;k<t.length;k++){if(t.events[k].judgment==4){assert(fabs(t.events[k].time-.88)<1e-6);early++;}if(t.events[k].judgment==5){assert(fabs(t.events[k].time-1.19)<1e-6);late++;}}
 assert(early==1&&late==1);free(t.events);assert(fabs(-.12+.07)<=.08);assert(fabs(-.19+.07)>.08&&fabs(-.19+.07)<.16);
 puts("PASS: Phira double precision 0.65 score, native rounding, Hold tail combo order, rule priority, type filter and accuracy goal");
}
