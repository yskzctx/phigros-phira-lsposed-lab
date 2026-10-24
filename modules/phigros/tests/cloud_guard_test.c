#define MODULE_HOST_TEST
#include "../src/cloud-guard.c"
#include <assert.h>
static BoolTask original_bool(const void *m){(void)m;return (BoolTask){.source=(void*)123,.result=true,.token=42};}
static IntTask original_int(const void *m){(void)m;return (IntTask){.source=(void*)456,.result=1,.token=17};}
int main(void){
 sync_original=original_bool;silent_original=original_int;
 atomic_store(&guard_ready,true);atomic_store(&isolated,false);
 BoolTask b=guard_sync(NULL);IntTask i=guard_silent(NULL);assert(b.source==(void*)123&&b.result&&b.token==42);assert(i.source==(void*)456&&i.result==1&&i.token==17);
 assert(activate_isolation(true));b=guard_sync(NULL);i=guard_silent(NULL);assert(b.source==(void*)123&&b.result&&b.token==42);assert(i.source==(void*)456&&i.result==1&&i.token==17);
 assert(activate_isolation(false));assert(guard_sync(NULL).source==(void*)123);
 atomic_store(&guard_ready,false);assert(!activate_isolation(true));assert(!atomic_load(&isolated));
 return 0;
}
