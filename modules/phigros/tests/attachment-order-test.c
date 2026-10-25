#include <assert.h>
#include <stdio.h>
#include "../native/attachment-order.h"
static int calls;static bool install(void *handle){assert(handle==(void*)1);calls++;return true;}
int main(void){
 AttachOrder before={0};attachment_library(&before,(void*)1,install);assert(calls==0);attachment_prepare(&before,install);assert(before.installed&&calls==1);attachment_library(&before,(void*)1,install);assert(calls==1);
 AttachOrder after={0};attachment_prepare(&after,install);assert(calls==1);attachment_library(&after,(void*)1,install);assert(after.installed&&calls==2);
 puts("PASS: library callback before/after storage preparation both attach exactly once; repeated callback does not rehook");
}
