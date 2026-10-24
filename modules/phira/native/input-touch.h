#ifndef LAB_INPUT_TOUCH_H
#define LAB_INPUT_TOUCH_H
#include "input-bridge.h"
#include <math.h>
#define LAB_MAX_CONTACTS 1024
typedef struct {void *line,*note;uint64_t id;} LabTouchContact;
typedef struct {
 LabInputApi input;
 void (*set_time)(void*,double);
 uint64_t (*line_position)(const void*,const void*,const void*,size_t);
 float (*line_rotation)(const void*,const void*,size_t);
 float (*note_x)(const void*);
 float (*screen_width)(void),(*screen_height)(void);
 double (*uptime)(void);
} LabTouchApi;
static inline bool lab_touch_submit(const LabTouchApi *api,const LabTouchContact *contacts,size_t count,void *resource,void *lines,size_t line_count,double time){
 if(count==0)return true;
 if(!api||!api->input.local_key||!api->input.repeat||!api->set_time||!api->line_position||!api->line_rotation||!api->note_x||!api->screen_width||!api->screen_height||!api->uptime||!contacts||!resource||!lines||count>LAB_MAX_CONTACTS||line_count>2048)return false;
 unsigned char *cell=api->input.local_key(NULL);if(!cell)return false;int64_t borrow;memcpy(&borrow,cell,8);if(borrow)return false;
 size_t existing;memcpy(&existing,cell+24,8);if(existing>1000000||count>(1000000-existing)/2)return false;
 float width=api->screen_width(),height=api->screen_height();if(!isfinite(width)||!isfinite(height)||width<=0||height<=0)return false;
 float points[LAB_MAX_CONTACTS][2];
 /* Match the original event-only fast-tap path: Started inserts a contact via
  * to_local(screen_width/screen_height), without the macroquad held-touch map.
  * Paired Ended events remove trackers; continuing Hold contacts repeat each
  * frame, within the original UP_TOLERANCE. No keyboard/unrated flag is added. */
 for(size_t i=0;i<line_count;i++)api->set_time((unsigned char*)lines+i*0x300+0xd0,time);
 for(size_t i=0;i<count;i++){
  const LabTouchContact *c=contacts+i;if(!c->line||!c->note)return false;
  api->set_time((unsigned char*)c->note+0x50,time);uint64_t packed=api->line_position(c->line,resource,lines,line_count);float pos[2];memcpy(pos,&packed,8);
  float radians=api->line_rotation(c->line,lines,line_count)*0.017453292519943295f,x=api->note_x((unsigned char*)c->note+0xf0);
  float wx=pos[0]+cosf(radians)*x,wy=pos[1]+sinf(radians)*x;
  /* Inverse of the original event-only to_local: y/height*2-1.
   * This fallback uses screen-down coordinates, not GL's inverted Y. */
  points[i][0]=(wx+1.f)*.5f*width;points[i][1]=(wy+1.f)*.5f*height;
  if(!isfinite(points[i][0])||!isfinite(points[i][1])||fabsf(points[i][0])>1e8f||fabsf(points[i][1])>1e8f)return false;
 }
 double now=api->uptime();if(!isfinite(now))return false;
 for(size_t i=0;i<count;i++){
  unsigned char event[32]={0};event[0]=7;event[1]=0;memcpy(event+4,points[i],8);memcpy(event+16,&contacts[i].id,8);memcpy(event+24,&now,8);
  api->input.repeat(event,NULL,cell+8);event[1]=2;api->input.repeat(event,NULL,cell+8);
 }
 return true;
}
#endif
