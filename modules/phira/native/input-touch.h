#ifndef LAB_INPUT_TOUCH_H
#define LAB_INPUT_TOUCH_H
#include "input-bridge.h"
#include <math.h>
#include <stdlib.h>
#define LAB_MAX_CONTACTS 20000
typedef struct {void *line,*note;uint64_t id;} LabTouchContact;
typedef struct {
 LabInputApi input;
 void (*set_time)(void*,double);
 uint64_t (*line_position)(const void*,const void*,const void*,size_t);
 float (*line_rotation)(const void*,const void*,size_t);
 float (*note_x)(const void*);
 float (*screen_width)(void),(*screen_height)(void);
} LabTouchApi;
typedef struct {unsigned char *cell;const LabTouchContact *contacts;size_t start,count;} LabTouchScope;
/* The original input handler stores Touch { id, position, time, phase } in a
 * Vec at RefCell+8. Remove only our appended range after Judge.update. UI and
 * pause handling must see the real input before/after it, never our contacts. */
static inline bool lab_touch_end(LabTouchScope *scope){
 if(!scope||!scope->cell)return true;
 int64_t borrow;memcpy(&borrow,scope->cell,8);if(borrow)return false;
 size_t cap,len;unsigned char *buffer;memcpy(&cap,scope->cell+8,8);memcpy(&buffer,scope->cell+16,8);memcpy(&len,scope->cell+24,8);
 size_t amount=scope->count*2;if(!buffer||cap>1000000||len>cap||scope->start>len||amount>len-scope->start)return false;
 for(size_t i=0;i<scope->count;i++)for(size_t phase=0;phase<2;phase++){
  unsigned char *event=buffer+(scope->start+i*2+phase)*32;uint64_t id;memcpy(&id,event,8);
  if(id!=scope->contacts[i].id||event[24]!=(phase?3:0))return false;
 }
 memmove(buffer+scope->start*32,buffer+(scope->start+amount)*32,(len-scope->start-amount)*32);len-=amount;memcpy(scope->cell+24,&len,8);memset(scope,0,sizeof(*scope));return true;
}
static inline bool lab_touch_submit_scoped(const LabTouchApi *api,const LabTouchContact *contacts,size_t count,void *resource,void *lines,size_t line_count,double time,LabTouchScope *scope){
 if(!scope||scope->cell)return false;
 if(count==0)return true;
 if(!api||!api->input.local_key||!api->input.repeat||!api->set_time||!api->line_position||!api->line_rotation||!api->note_x||!api->screen_width||!api->screen_height||!contacts||!resource||!lines||count>LAB_MAX_CONTACTS||line_count>2048)return false;
 unsigned char *cell=api->input.local_key(NULL);if(!cell)return false;int64_t borrow;memcpy(&borrow,cell,8);if(borrow)return false;
 size_t existing,capacity;memcpy(&existing,cell+24,8);memcpy(&capacity,cell+8,8);if(capacity>1000000||existing>capacity||count>(1000000-existing)/2)return false;
 float width=api->screen_width(),height=api->screen_height();if(!isfinite(width)||!isfinite(height)||width<=0||height<=0)return false;
 float (*points)[2]=malloc(count*sizeof(*points));if(!points)return false;bool okay=false;
 /* Match the original event-only fast-tap path: Started inserts a contact via
  * to_local(screen_width/screen_height), without the macroquad held-touch map.
  * Paired Ended events remove trackers; continuing Hold contacts repeat each
  * frame, within the original UP_TOLERANCE. No keyboard/unrated flag is added. */
 for(size_t i=0;i<line_count;i++)api->set_time((unsigned char*)lines+i*0x300+0xd0,time);
 for(size_t i=0;i<count;i++){
  const LabTouchContact *c=contacts+i;if(!c->line||!c->note)goto done;
  api->set_time((unsigned char*)c->note+0x50,time);uint64_t packed=api->line_position(c->line,resource,lines,line_count);float pos[2];memcpy(pos,&packed,8);
  float radians=api->line_rotation(c->line,lines,line_count)*0.017453292519943295f,x=api->note_x((unsigned char*)c->note+0xf0);
  float wx=pos[0]+cosf(radians)*x,wy=pos[1]+sinf(radians)*x;
  /* Original Judge does to_local, then inverse_line(Point(p.x, -p.y)).
   * Invert BOTH stages. Omitting the second Y reversal misses moving lines. */
  points[i][0]=(wx+1.f)*.5f*width;points[i][1]=(1.f-wy)*.5f*height;
  if(!isfinite(points[i][0])||!isfinite(points[i][1])||fabsf(points[i][0])>1e8f||fabsf(points[i][1])>1e8f)goto done;
 }
 /* The original Judge explicitly supports infinite-time contacts (its mouse
  * path uses -INFINITY), resolved at Resource.time. Generated chart events
  * must not drift early while a dense batch is being constructed/processed. */
 double now=-INFINITY;
 scope->cell=cell;scope->contacts=contacts;scope->start=existing;scope->count=count;
 for(size_t i=0;i<count;i++){
  unsigned char event[32]={0};event[0]=7;event[1]=0;memcpy(event+4,points[i],8);memcpy(event+16,&contacts[i].id,8);memcpy(event+24,&now,8);
  api->input.repeat(event,NULL,cell+8);event[1]=2;api->input.repeat(event,NULL,cell+8);
 }
 okay=true;
done:
 free(points);return okay;
}
#endif
