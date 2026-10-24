#ifndef LAB_INPUT_BRIDGE_H
#define LAB_INPUT_BRIDGE_H
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
/* Verified against original ARM64 event repeat at 0xd8e4ac, not a score API.
 * KeyCode::Space and all modifier/repeat booleans are zero in the pinned source.
 * Never pass a key event through a wheel/touch variant or touch score storage. */
typedef struct {void *(*local_key)(void *);void (*repeat)(const void *,void *,void *);} LabInputApi;
typedef struct {bool held;} LabInputOwner;
static inline bool lab_input_apply(const LabInputApi *api,LabInputOwner *owner,unsigned presses,bool held){
 if(!api||!api->local_key||!api->repeat||!owner||presses>1024)return false;
 unsigned char *cell=api->local_key(NULL);if(!cell)return false;int64_t borrow;memcpy(&borrow,cell,8);if(borrow)return false;
 unsigned char *status=cell+8;uint32_t original_down;int32_t original_delta;memcpy(&original_down,status+28,4);memcpy(&original_delta,status+24,4);
 if(original_down>UINT32_MAX-presses-2u||original_delta>INT32_MAX-2||original_delta<INT32_MIN+2)return false;
 unsigned char event[32]={0};unsigned pending=presses;
 if(owner->held&&!held){event[0]=6;api->repeat(event,NULL,status);}
 if(!owner->held&&held){
  event[0]=5;api->repeat(event,NULL,status);
  if(pending)pending--;else{
   /* Initial contact holds Drag/Flick/Hold without an unwanted Tap judgement.
    * Subtract just this module's newly added press, preserving real input. */
   uint32_t now;memcpy(&now,status+28,4);now--;memcpy(status+28,&now,4);
  }
 }
 for(unsigned i=0;i<pending;i++){
  if(held){event[0]=6;api->repeat(event,NULL,status);event[0]=5;api->repeat(event,NULL,status);}
  else{event[0]=5;api->repeat(event,NULL,status);event[0]=6;api->repeat(event,NULL,status);}
 }
 owner->held=held;return true;
}
#endif
