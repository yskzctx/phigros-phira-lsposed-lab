#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <string.h>
// Exact fields from content157 il2cpp.h. Null source means a completed
// UniTask whose result is stored inline, rather than an unfinished promise.
typedef struct {void *source;bool result;int16_t token;} BoolTask;
typedef struct {void *source;int32_t result;int16_t token;} IntTask;
_Static_assert(sizeof(BoolTask)==16&&offsetof(BoolTask,result)==8&&offsetof(BoolTask,token)==10,"bool task ABI");
_Static_assert(sizeof(IntTask)==16&&offsetof(IntTask,result)==8&&offsetof(IntTask,token)==12,"enum task ABI");
static BoolTask (*sync_original)(const void *method);
static IntTask (*silent_original)(const void *method);
static atomic_bool guard_ready=false,isolated=false;
static BoolTask guard_sync(const void *method){
 return sync_original(method);
}
static IntTask guard_silent(const void *method){
 return silent_original(method);
}
static bool activate_isolation(bool value){
 if(value&&!atomic_load(&guard_ready))return false;
 atomic_store(&isolated,value);return true;
}
#ifndef MODULE_HOST_TEST
#include <jni.h>
#include <dlfcn.h>
#include <link.h>
#include <pthread.h>
#include <android/log.h>
#include "cloud-profile.h"
typedef int(*HookFunType)(void*,void*,void**);
typedef int(*UnhookFunType)(void*);
typedef void(*NativeOnModuleLoaded)(const char*,void*);
typedef struct{uint32_t version;HookFunType hook_func;UnhookFunType unhook_func;} NativeAPIEntries;
static HookFunType hook_func;
static pthread_mutex_t lock=PTHREAD_MUTEX_INITIALIZER;
typedef struct{uintptr_t base;bool valid[2];} RangeCheck;
static int executable_ranges(struct dl_phdr_info *info,size_t size,void *context){
 (void)size;RangeCheck *r=context;if(info->dlpi_addr!=r->base)return 0;
 for(size_t i=0;i<info->dlpi_phnum;i++){
  const ElfW(Phdr)*p=&info->dlpi_phdr[i];if(p->p_type!=PT_LOAD||!(p->p_flags&PF_X))continue;
  for(size_t k=0;k<2;k++)if(cloud_hooks[k].rva>=p->p_vaddr&&cloud_hooks[k].rva-p->p_vaddr<=p->p_memsz&&p->p_memsz-(cloud_hooks[k].rva-p->p_vaddr)>=16)r->valid[k]=true;
 }return 1;
}
static void library_loaded(const char *name,void *handle){
 if(!name||!handle||!hook_func)return;
 const char *leaf=strrchr(name,'/');leaf=leaf?leaf+1:name;if(strcmp(leaf,"libil2cpp.so"))return;
 pthread_mutex_lock(&lock);if(sync_original||silent_original){pthread_mutex_unlock(&lock);return;}
 Dl_info info;void *export=dlsym(handle,"il2cpp_domain_get");
 if(!export||!dladdr(export,&info)||!info.dli_fbase){pthread_mutex_unlock(&lock);return;}
 RangeCheck ranges={.base=(uintptr_t)info.dli_fbase};dl_iterate_phdr(executable_ranges,&ranges);
 for(size_t k=0;k<2;k++)if(!ranges.valid[k]||memcmp((void*)(ranges.base+cloud_hooks[k].rva),cloud_hooks[k].prefix,16)){
  __android_log_print(ANDROID_LOG_ERROR,"PhigrosLab","Cloud guard profile mismatch; isolation cannot activate");pthread_mutex_unlock(&lock);return;
 }
 if(hook_func((void*)(ranges.base+cloud_hooks[0].rva),(void*)guard_sync,(void**)&sync_original)==0&&
    hook_func((void*)(ranges.base+cloud_hooks[1].rva),(void*)guard_silent,(void**)&silent_original)==0){
  atomic_store(&guard_ready,true);
 }else{__android_log_print(ANDROID_LOG_ERROR,"PhigrosLab","Cloud guard incomplete; isolation remains disabled");}
 pthread_mutex_unlock(&lock);
}
__attribute__((visibility("default"),used)) NativeOnModuleLoaded native_init(const NativeAPIEntries *entries){
 if(!entries||!entries->hook_func)return NULL;hook_func=entries->hook_func;return library_loaded;
}
JNIEXPORT jboolean JNICALL Java_com_phigros_lab_LabGuard_isReady(JNIEnv *env,jclass clazz){(void)env;(void)clazz;return atomic_load(&guard_ready)?JNI_TRUE:JNI_FALSE;}
JNIEXPORT jboolean JNICALL Java_com_phigros_lab_LabGuard_setIsolated(JNIEnv *env,jclass clazz,jboolean value){(void)env;(void)clazz;return activate_isolation(value==JNI_TRUE)?JNI_TRUE:JNI_FALSE;}
#endif
