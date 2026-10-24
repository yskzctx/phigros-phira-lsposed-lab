#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../native/controlled-frame.h"
static unsigned char cell[64];
static void *key(void *unused){(void)unused;return cell;}
static void repeat(const void *event,void *unused,void *handler){(void)unused;unsigned char *p=handler;int32_t d;uint32_t n;memcpy(&d,p+24,4);memcpy(&n,p+28,4);if(*(const unsigned char*)event==5){d++;n++;}else d--;memcpy(p+24,&d,4);memcpy(p+28,&n,4);}
static void ptr(void *p,size_t at,void *value){memcpy((char*)p+at,&value,8);}
static void count(void *p,size_t at,size_t value){memcpy((char*)p+at,&value,8);}
static void real(void *p,size_t at,double value){memcpy((char*)p+at,&value,8);}
static int touch_events;
static void touch_repeat(const void *event,void *ctx,void *status){assert(!ctx&&status==cell+8&&*(const unsigned char*)event==7);touch_events++;}
static void set_time(void *p,double t){assert(p&&isfinite(t));}
static uint64_t line_pos(const void *l,const void *r,const void *ls,size_t n){assert(l&&r&&ls&&n==1);return 0;}
static float line_rot(const void *l,const void *ls,size_t n){assert(l&&ls&&n==1);return 0;}
static float x_now(const void *p){assert(p);return 0;}static float width(void){return 960;}static float height(void){return 540;}static double uptime(void){return 1000;}
int main(void){
 unsigned char judge[512]={0},chart[256]={0},resource[2600]={0},line[0x300]={0},notes[3*0x188]={0},rows[32]={0};uint32_t ids[3]={0,1,2};
 memset(judge+0x58,0xa5,0x120);unsigned char protected_before[0x120];memcpy(protected_before,judge+0x58,sizeof(protected_before));
 ptr(judge,8,rows);count(judge,16,1);ptr(chart,0x78,line);count(chart,0x80,1);ptr(line,0x1c8,notes);count(line,0x1d0,3);ptr(rows,8,ids);count(rows,16,3);count(rows,24,0);
 for(int i=0;i<3;i++){count(notes+i*0x188,0,i);real(notes+i*0x188,0x140,i+1);}real(notes+0x188,8,4);float speed=1;memcpy(resource+0x9c,&speed,4);
 ControlConfig cfg={.rule_count=3};cfg.rules[0]=(NoteRule){.judgment=4,.position=1,.start=1};cfg.rules[1]=(NoteRule){.judgment=1,.position=1,.start=2};cfg.rules[2]=(NoteRule){.judgment=3,.position=1,.start=3};LabPlan plan={0};LabInputApi api={key,repeat};
 real(resource,0x9e0,.89);assert(lab_frame_begin(&plan,judge,resource,chart,true,&cfg,123,&api));assert(plan.controlled&&plan.fired==1);assert(fabs(lab_f64(notes,0x140)-1.01)<1e-7);notes[0x30]=2;lab_frame_end(&plan);assert(lab_f64(notes,0x140)==1&&ids[0]==0&&lab_size(rows,24)==1);
 memset(cell+32,0,8);real(resource,0x9e0,2.10);assert(lab_frame_begin(&plan,judge,resource,chart,true,&cfg,123,&api));assert(plan.fired==0&&lab_f64(notes+0x188,0x140)>2.32);lab_frame_end(&plan);assert(lab_f64(notes+0x188,0x140)==2);
 real(resource,0x9e0,2.23);assert(lab_frame_begin(&plan,judge,resource,chart,true,&cfg,123,&api));assert(plan.expired_miss&&plan.fired==0&&lab_f64(notes+0x188,0x140)==2);notes[0x188+0x30]=2;lab_frame_end(&plan);
 real(resource,0x9e0,2.82);assert(lab_frame_begin(&plan,judge,resource,chart,true,&cfg,123,&api));assert(plan.fired==1&&lab_size(notes+2*0x188,0)==0);notes[2*0x188+0x30]=1;lab_frame_end(&plan);assert(notes[2*0x188+0x30]==2);assert(lab_size(notes+2*0x188,0)==2&&lab_f64(notes+2*0x188,0x140)==3);
 assert(!memcmp(protected_before,judge+0x58,sizeof(protected_before)));assert(lab_size(rows,24)==3);
 uint64_t generation=plan.generation;for(int k=0;k<3;k++)notes[k*0x188+0x30]=0;real(resource,0x9e0,2.83);assert(lab_frame_begin(&plan,judge,resource,chart,true,&cfg,123,&api));assert(plan.generation>generation);lab_frame_end(&plan);
 count(chart,0x80,2);assert(!lab_frame_begin(&plan,judge,resource,chart,true,&cfg,123,&api));assert(!memcmp(protected_before,judge+0x58,sizeof(protected_before)));
 lab_plan_release(&plan);count(chart,0x80,1);for(int k=0;k<3;k++)notes[k*0x188+0x30]=0;count(rows,24,0);memset(cell,0,sizeof(cell));resource[0xbc]=0;resource[0xb7]=1;uint32_t mods=0x23;memcpy(resource+0x88,&mods,4);real(resource,0x9e0,.89);
 LabTouchApi touch={.input={key,touch_repeat},.set_time=set_time,.line_position=line_pos,.line_rotation=line_rot,.note_x=x_now,.screen_width=width,.screen_height=height,.uptime=uptime};plan.touch=&touch;
 assert(lab_frame_begin(&plan,judge,resource,chart,true,&cfg,123,&api));assert(touch_events==2&&resource[0xbc]==0&&resource[0xb7]==1);memcpy(&mods,resource+0x88,4);assert(mods==0x22);lab_frame_end(&plan);memcpy(&mods,resource+0x88,4);assert(mods==0x23);
 int64_t borrowed=1;memcpy(cell,&borrowed,8);assert(!lab_frame_begin(&plan,judge,resource,chart,true,&cfg,123,&api));assert(lab_f64(notes,0x140)==1&&lab_size(notes+2*0x188,0)==2&&resource[0xbc]==0);memcpy(&mods,resource+0x88,4);assert(mods==0x23);
 unsigned char unchanged[2600];memcpy(unchanged,resource,sizeof(resource));assert(lab_frame_begin(&plan,judge,resource,chart,false,&cfg,123,&api));lab_frame_end(&plan);assert(!memcmp(unchanged,resource,sizeof(resource)));
 assert(!memcmp(protected_before,judge+0x58,sizeof(protected_before)));lab_plan_release(&plan);
 memset(cell,0,sizeof(cell));for(int k=0;k<3;k++)notes[k*0x188+0x30]=0;count(rows,24,0);plan.touch=&touch;cfg.rules[0].judgment=5;
 real(resource,0x9e0,1.13);assert(lab_frame_begin(&plan,judge,resource,chart,true,&cfg,123,&api));assert(plan.fired==0);lab_frame_end(&plan);
 real(resource,0x9e0,1.20);assert(lab_frame_begin(&plan,judge,resource,chart,true,&cfg,123,&api));assert(plan.fired==1);double late_delta=lab_f64(notes,0x140)-1.20;assert(fabs(late_delta+.19)<1e-7);assert(fabs(late_delta+.07)>.08&&fabs(late_delta+.07)<.16);lab_frame_end(&plan);assert(lab_f64(notes,0x140)==1);
 assert(!memcmp(protected_before,judge+0x58,sizeof(protected_before)));lab_plan_release(&plan);puts("PASS: Early/Late Good including native early-offset compensation, natural Miss, Bad lifecycle, retry/index/time/kind rollback, touch integration and protected-byte canary");
}
