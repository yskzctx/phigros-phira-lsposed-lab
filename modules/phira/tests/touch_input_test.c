#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../native/input-touch.h"
static unsigned char cell[64],line[0x300],note[0x188];static int events;static unsigned char sent[8][32],vector[8192*32];
static void *key(void *p){assert(!p);return cell;}
static void repeat(const void *event,void *ctx,void *status){assert(!ctx&&status==cell+8);if(events<8)memcpy(sent[events],event,32);events++;size_t len;memcpy(&len,cell+24,8);assert(len<8192);const unsigned char *in=event;unsigned char *out=vector+len*32;memset(out,0,32);memcpy(out,in+16,8);memcpy(out+8,in+4,8);memcpy(out+16,in+24,8);out[24]=in[1]==0?0:3;len++;memcpy(cell+24,&len,8);}
static void clock_at(void *object,double time){assert(object==line+0xd0||object==note+0x50);assert(time==3.25);}
static float origin[2]={.2f,.1f},angle=90;
static uint64_t position(const void *l,const void *r,const void *ls,size_t n){assert(l==line&&r&&ls==line&&n==1);uint64_t out;memcpy(&out,origin,8);return out;}
static float rotation(const void *l,const void *ls,size_t n){assert(l==line&&ls==line&&n==1);return angle;}
static float note_x(const void *a){assert(a==note+0xf0);return .4f;}
static float width(void){return 2000;}static float height(void){return 1000;}
int main(void){
 unsigned char resource[2600]={0};LabTouchApi api={.input={key,repeat},.set_time=clock_at,.line_position=position,.line_rotation=rotation,.note_x=note_x,.screen_width=width,.screen_height=height};LabTouchContact contact={.line=line,.note=note,.id=0x5048495241000001ULL};
 size_t cap=8192,len=1;void *buffer=vector;memcpy(cell+8,&cap,8);memcpy(cell+16,&buffer,8);memcpy(cell+24,&len,8);uint64_t physical=123;memcpy(vector,&physical,8);unsigned char original[32];memcpy(original,vector,32);LabTouchScope scope={0};
 assert(lab_touch_submit_scoped(&api,&contact,1,resource,line,1,3.25,&scope));assert(events==2&&sent[0][0]==7&&sent[0][1]==0&&sent[1][1]==2);float x,y;memcpy(&x,sent[0]+4,4);memcpy(&y,sent[0]+8,4);assert(fabsf(x-1200)<.01&&fabsf(y-250)<.01);
 // Complete the actual judge pipeline: to_local, Y inversion, inverse line
 // rotation/translation. It must land at note-local (0.4, 0), not (-0.6, 0).
 float wx=x/2000*2-1,wy=-(y/1000*2-1);float local_x=wy-.1f,local_y=-(wx-.2f);assert(fabsf(local_x-.4f)<1e-6&&fabsf(local_y)<1e-6);
 uint64_t id;double stamp;memcpy(&id,sent[0]+16,8);memcpy(&stamp,sent[0]+24,8);assert(id==contact.id&&isinf(stamp)&&stamp<0);assert(!memcmp(resource,(unsigned char[2600]){0},2600));
 uint64_t later=456;memcpy(vector+3*32,&later,8);len=4;memcpy(cell+24,&len,8);assert(lab_touch_end(&scope));memcpy(&len,cell+24,8);assert(len==2&&!memcmp(vector,original,32));memcpy(&id,vector+32,8);assert(id==later);assert(!scope.cell);
 int64_t borrowed=1;memcpy(cell,&borrowed,8);assert(!lab_touch_submit_scoped(&api,&contact,1,resource,line,1,3.25,&scope));assert(events==2);
 borrowed=0;memcpy(cell,&borrowed,8);LabTouchContact dense[2048];for(int i=0;i<2048;i++){dense[i]=contact;dense[i].id+=(uint64_t)i;}assert(lab_touch_submit_scoped(&api,dense,2048,resource,line,1,3.25,&scope));assert(lab_touch_end(&scope));memcpy(&len,cell+24,8);assert(len==2);
 float angles[]={-180,-90,-45,0,30,90,179};for(size_t k=0;k<sizeof(angles)/sizeof(angles[0]);k++){
  angle=angles[k];origin[0]=-.5f;origin[1]=.3f;events=0;assert(lab_touch_submit_scoped(&api,&contact,1,resource,line,1,3.25,&scope));memcpy(&x,sent[0]+4,4);memcpy(&y,sent[0]+8,4);wx=x/2000*2-1;wy=-(y/1000*2-1);float r=angle*.017453292519943295f,dx=wx-origin[0],dy=wy-origin[1];assert(fabsf(cosf(r)*dx+sinf(r)*dy-.4f)<1e-5&&fabsf(-sinf(r)*dx+cosf(r)*dy)<1e-5);assert(lab_touch_end(&scope));
 }
 assert(lab_touch_submit_scoped(&api,&contact,1,resource,line,1,3.25,&scope));borrowed=1;memcpy(cell,&borrowed,8);assert(!lab_touch_end(&scope)&&scope.cell);borrowed=0;memcpy(cell,&borrowed,8);assert(lab_touch_end(&scope));
 int before_events=events;origin[0]=NAN;assert(!lab_touch_submit_scoped(&api,&contact,1,resource,line,1,3.25,&scope));assert(!scope.cell&&events==before_events);memcpy(&len,cell+24,8);assert(len==2);
 puts("PASS: full rotated judge-coordinate pipeline, virtual chart-time contacts, 2048 dense contacts, scoped cleanup and real touches preserved");
}
