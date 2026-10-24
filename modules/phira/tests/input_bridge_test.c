#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../native/input-bridge.h"
static unsigned char cell[64];static int calls;
static void *local(void *unused){assert(!unused);return cell;}
static void repeat(const void *event,void *ctx,void *handler){assert(!ctx);assert(handler==cell+8);const unsigned char *e=event;int32_t d;uint32_t n;memcpy(&d,cell+32,4);memcpy(&n,cell+36,4);if(e[0]==5&&!e[6]){d++;n++;}else if(e[0]==6)d--;else assert(0);memcpy(cell+32,&d,4);memcpy(cell+36,&n,4);calls++;}
static int32_t delta(void){int32_t d;memcpy(&d,cell+32,4);return d;}
static uint32_t down(void){uint32_t n;memcpy(&n,cell+36,4);return n;}
int main(void){
 memset(cell,0,sizeof(cell));memset(cell+8,0xa5,24);memset(cell+40,0x5a,24);unsigned char before[64];memcpy(before,cell,64);
 LabInputApi api={local,repeat};LabInputOwner own={0};
 assert(lab_input_apply(&api,&own,0,true));assert(own.held&&delta()==1&&down()==0);
 assert(lab_input_apply(&api,&own,3,true));assert(delta()==1&&down()==3);
 assert(lab_input_apply(&api,&own,0,false));assert(!own.held&&delta()==0&&down()==3);
 assert(lab_input_apply(&api,&own,2,false));assert(delta()==0&&down()==5);
 assert(!memcmp(cell+8,before+8,24)&&!memcmp(cell+40,before+40,24));
 int n=calls;int64_t borrowed=1;memcpy(cell,&borrowed,8);assert(!lab_input_apply(&api,&own,1,true));assert(calls==n&&!own.held);
 borrowed=0;memcpy(cell,&borrowed,8);assert(!lab_input_apply(&api,&own,1025,true));assert(calls==n);
 puts("PASS: synthetic input ownership, no phantom initial note, press/release deltas, real input preserved, borrowed TLS refused");
}
