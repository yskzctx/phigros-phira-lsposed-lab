#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../native/input-touch.h"
static unsigned char cell[64],line[0x300],note[0x188];static int events;static unsigned char sent[4][32];
static void *key(void *p){assert(!p);return cell;}
static void repeat(const void *event,void *ctx,void *status){assert(!ctx&&status==cell+8);assert(events<4);memcpy(sent[events++],event,32);}
static void clock_at(void *object,double time){assert(object==line+0xd0||object==note+0x50);assert(time==3.25);}
static uint64_t position(const void *l,const void *r,const void *ls,size_t n){assert(l==line&&r&&ls==line&&n==1);float p[2]={.2f,.1f};uint64_t out;memcpy(&out,p,8);return out;}
static float rotation(const void *l,const void *ls,size_t n){assert(l==line&&ls==line&&n==1);return 90;}
static float note_x(const void *a){assert(a==note+0xf0);return .4f;}
static float width(void){return 2000;}static float height(void){return 1000;}static double uptime(void){return 12345.25;}
int main(void){
 unsigned char resource[2600]={0};LabTouchApi api={.input={key,repeat},.set_time=clock_at,.line_position=position,.line_rotation=rotation,.note_x=note_x,.screen_width=width,.screen_height=height,.uptime=uptime};LabTouchContact contact={.line=line,.note=note,.id=0x5048495241000001ULL};
 assert(lab_touch_submit(&api,&contact,1,resource,line,1,3.25));assert(events==2&&sent[0][0]==7&&sent[0][1]==0&&sent[1][1]==2);float x,y;memcpy(&x,sent[0]+4,4);memcpy(&y,sent[0]+8,4);assert(fabsf(x-1200)<.01&&fabsf(y-250)<.01);uint64_t id;double stamp;memcpy(&id,sent[0]+16,8);memcpy(&stamp,sent[0]+24,8);assert(id==contact.id&&stamp==12345.25);assert(!memcmp(resource,(unsigned char[2600]){0},2600));
 int64_t borrowed=1;memcpy(cell,&borrowed,8);assert(!lab_touch_submit(&api,&contact,1,resource,line,1,3.25));assert(events==2);
 puts("PASS: rotated world-to-window contact, original Touch start/end event layout, finite native clock, no keyboard/network/score flags changed");
}
