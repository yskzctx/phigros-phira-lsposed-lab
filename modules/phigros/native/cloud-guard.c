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
static atomic_int sync_calls,silent_calls;
static atomic_bool last_sync_result;
// Forwarding observers. They exist so the module can *report* whether the host's
// own sync path is being reached at all, which is the only way to tell "the module
// broke sync" apart from "the host never called it". They never change a value:
// the module keeps the host's login, network and upload flow intact.
static BoolTask guard_sync(const void *method){
 atomic_fetch_add(&sync_calls,1);
 BoolTask task=sync_original(method);
 atomic_store(&last_sync_result,task.result);
 return task;
}
static IntTask guard_silent(const void *method){
 atomic_fetch_add(&silent_calls,1);
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
#ifndef CLOUD_GUARD_INCLUDED
/* When inlined into lsp-entry.c the caller already defines NativeAPIEntries with
 * these exact members, so redeclaring it would be a typedef redefinition. */
typedef struct{uint32_t version;HookFunType hook_func;UnhookFunType unhook_func;} NativeAPIEntries;
#endif
static HookFunType hook_func;
static UnhookFunType unhook_func;
static pthread_mutex_t lock=PTHREAD_MUTEX_INITIALIZER;
typedef struct{uintptr_t base;bool valid[2];} RangeCheck;
static int executable_ranges(struct dl_phdr_info *info,size_t size,void *context){
 (void)size;RangeCheck *r=context;if(info->dlpi_addr!=r->base)return 0;
 for(size_t i=0;i<info->dlpi_phnum;i++){
  const ElfW(Phdr)*p=&info->dlpi_phdr[i];if(p->p_type!=PT_LOAD||!(p->p_flags&PF_X))continue;
  for(size_t k=0;k<2;k++)if(cloud_hooks[k].rva>=p->p_vaddr&&cloud_hooks[k].rva-p->p_vaddr<=p->p_memsz&&p->p_memsz-(cloud_hooks[k].rva-p->p_vaddr)>=16)r->valid[k]=true;
 }return 1;
}
static void install_cloud_hooks(const char *name,void *handle){
 if(!name||!handle||!hook_func)return;
 const char *leaf=strrchr(name,'/');leaf=leaf?leaf+1:name;if(strcmp(leaf,"libil2cpp.so"))return;
 pthread_mutex_lock(&lock);
 if(sync_original||silent_original){pthread_mutex_unlock(&lock);return;}
 Dl_info info;void *export=dlsym(handle,"il2cpp_domain_get");
 if(!export||!dladdr(export,&info)||!info.dli_fbase){pthread_mutex_unlock(&lock);return;}
 RangeCheck ranges={.base=(uintptr_t)info.dli_fbase};dl_iterate_phdr(executable_ranges,&ranges);
 for(size_t k=0;k<2;k++)if(!ranges.valid[k]||memcmp((void*)(ranges.base+cloud_hooks[k].rva),cloud_hooks[k].prefix,16)){
  __android_log_print(ANDROID_LOG_ERROR,"PhigrosLab","Cloud observer profile mismatch; observers stay off, host sync untouched");pthread_mutex_unlock(&lock);return;
 }
 if(hook_func((void*)(ranges.base+cloud_hooks[0].rva),(void*)guard_sync,(void**)&sync_original)==0&&
    hook_func((void*)(ranges.base+cloud_hooks[1].rva),(void*)guard_silent,(void**)&silent_original)==0){
  atomic_store(&guard_ready,true);
  __android_log_print(ANDROID_LOG_INFO,"PhigrosLab","Cloud observers attached; host sync/upload unchanged");
 }else{
  // Roll back a half-installed pair so the host is never left with one hook.
  if(sync_original&&unhook_func)unhook_func((void*)(ranges.base+cloud_hooks[0].rva));
  sync_original=NULL;silent_original=NULL;
  __android_log_print(ANDROID_LOG_ERROR,"PhigrosLab","Cloud observers incomplete; rolled back, host sync untouched");
 }
 pthread_mutex_unlock(&lock);
}
static void library_loaded(const char *name,void *handle){install_cloud_hooks(name,handle);}
/* Called by lsp-entry.c's single native_init so the observers share the module's
 * one hook API handle. Safe to call before or after libil2cpp is mapped. */
static void cloud_hook_api(void *hook,void *unhook){
 if(!hook)return;
 hook_func=(HookFunType)hook;unhook_func=(UnhookFunType)unhook;
 void *existing=dlopen("libil2cpp.so",RTLD_NOW|RTLD_NOLOAD);
 if(existing){install_cloud_hooks("libil2cpp.so",existing);dlclose(existing);}
}
/* Included from lsp-entry.c, which owns the single exported native_init. Defining
 * a second one here would collide at link time, so the entry point is only built
 * when this file is compiled standalone (its host test). */
#ifndef CLOUD_GUARD_INCLUDED
__attribute__((visibility("default"),used)) NativeOnModuleLoaded native_init(const NativeAPIEntries *entries){
 if(!entries||!entries->hook_func)return NULL;hook_func=entries->hook_func;unhook_func=entries->unhook_func;return library_loaded;
}
#endif
JNIEXPORT jboolean JNICALL Java_com_phigros_lab_LabGuard_isReady(JNIEnv *env,jclass clazz){(void)env;(void)clazz;return atomic_load(&guard_ready)?JNI_TRUE:JNI_FALSE;}
JNIEXPORT jboolean JNICALL Java_com_phigros_lab_LabGuard_setIsolated(JNIEnv *env,jclass clazz,jboolean value){(void)env;(void)clazz;return activate_isolation(value==JNI_TRUE)?JNI_TRUE:JNI_FALSE;}
/** 自检：把观察到的同步调用次数与最近一次结果回给面板，用来区分“模块坏了”和“宿主没调用”。 */
JNIEXPORT jintArray JNICALL Java_com_phigros_lab_LabGuard_diagnostics(JNIEnv *env,jclass clazz){
 (void)clazz;
 jint values[]={atomic_load(&guard_ready),atomic_load(&isolated),atomic_load(&sync_calls),atomic_load(&silent_calls),atomic_load(&last_sync_result)};
 jintArray out=(*env)->NewIntArray(env,5);if(out)(*env)->SetIntArrayRegion(env,out,0,5,values);return out;
}
#endif
