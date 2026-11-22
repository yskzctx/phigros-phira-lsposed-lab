// Runtime attachment only. Never modifies the installed game's APK or signature.
#define JNI_OnLoad offline_unused_onload
#include "core/offline.c"
#undef JNI_OnLoad
// 4.1.0 hook table. Same shape as the 157 table in core/expected-hooks.h but with the
// offsets recovered for the 4.1.0 build; the entry point below picks whichever table's
// prologues all match the loaded libil2cpp.so, so one module serves both versions.
#include "core/expected-hooks-161.h"
#include "../../common/hook-batch.h"
#include "attachment-order.h"
#include <dlfcn.h>

// Cloud observers live in their own translation unit for the host test, but are
// included here so this file stays the single owner of the exported native_init.
// CLOUD_GUARD_INCLUDED suppresses cloud-guard's own entry point.
#define CLOUD_GUARD_INCLUDED 1
#include "cloud-guard.c"
#undef CLOUD_GUARD_INCLUDED

typedef void (*NativeOnModuleLoaded)(const char*,void*);
typedef struct{uint32_t version;LabHook hook_func;LabUnhook unhook_func;} NativeAPIEntries;
static LabBatch batch;
static AttachOrder attachment;
static pthread_mutex_t attachment_lock=PTHREAD_MUTEX_INITIALIZER;
static atomic_int connection_stage,failed_hook=-1,hook_error;
static atomic_bool api_received;
// The table in use for this launch, plus the Bad-judgment helper that sits with it.
// 157 and 4.1.0 keep bad_call at different offsets, so it travels with the table.
static const struct HookSpec *active_hooks;
static uintptr_t active_bad_call;
static const struct HookSpec *const candidate_tables[]={hooks,hooks161};
static const uintptr_t candidate_bad_call[]={0x1d875c4,0x1d89dcc};
#define TABLE_COUNT (sizeof(candidate_tables)/sizeof(candidate_tables[0]))
// Is this RVA inside an executable segment of the loaded image?
static bool in_exec(const struct dl_phdr_info *info,uintptr_t target){
 for(size_t i=0;i<info->dlpi_phnum;i++){const ElfW(Phdr)*p=&info->dlpi_phdr[i];
  if(p->p_type==PT_LOAD&&(p->p_flags&PF_X)&&target>=p->p_vaddr&&target-p->p_vaddr<=p->p_memsz&&p->p_memsz-(target-p->p_vaddr)>=16)return true;
 }return false;
}
static int select_hook_table(struct dl_phdr_info *info,size_t size,void *data){
 (void)size;uintptr_t *base=data;if(info->dlpi_addr!=*base)return 0;
 // Pick the first candidate table whose every prologue matches the loaded code. A
 // mismatched table means this build is a different game version, which is exactly how
 // the 157 table falls through to the 4.1.0 table on a 4.1.0 install.
 for(size_t t=0;t<TABLE_COUNT;t++){
  const struct HookSpec *table=candidate_tables[t];
  bool all=true;
  for(size_t k=0;k<9;k++){
   uintptr_t at=table[k].rva;
   if(!in_exec(info,at)||memcmp((void*)(info->dlpi_addr+at),table[k].prefix,16)){all=false;break;}
  }
  if(all){active_hooks=table;active_bad_call=candidate_bad_call[t];
   __android_log_print(ANDROID_LOG_INFO,"PhigrosLab","hook table %zu selected",t);return 1;}
 }
 return 1;
}
static bool install_library(void *handle){
 if(atomic_load(&ready))return true;
 if(!handle||!batch.hook||!batch.unhook||atomic_load(&connection_stage)<0)return false;
 atomic_store(&connection_stage,3);
 Dl_info location;void *symbol=dlsym(handle,"il2cpp_domain_get");if(!symbol||!dladdr(symbol,&location)||!location.dli_fbase){atomic_store(&connection_stage,-1);return false;}
 image=(uintptr_t)location.dli_fbase;dl_iterate_phdr(select_hook_table,&image);
 if(!active_hooks){atomic_store(&connection_stage,-2);return false;}
 const struct HookSpec *table=active_hooks;
 for(size_t i=0;i<9;i++){
  uintptr_t at=table[i].rva;
  const unsigned char *prefix=table[i].prefix;
  if(memcmp((void*)(image+at),prefix,16)){atomic_store(&failed_hook,(int)i);atomic_store(&connection_stage,-2);return false;}
 }
 bad_call=(void(*)(void*,float,float,const void*))(image+active_bad_call);
 void *replace[]={(void*)click,(void*)drag,(void*)flick,(void*)hold,(void*)record,(void*)perfect,(void*)result,(void*)miss,(void*)good};
 void **backup[]={(void**)&click_original,(void**)&drag_original,(void**)&flick_original,(void**)&hold_original,(void**)&record_original,(void**)&perfect_original,(void**)&result_original,(void**)&miss_original,(void**)&good_call};
 for(size_t i=0;i<9;i++){
  uintptr_t at=table[i].rva;int status=batch_install(&batch,(void*)(image+at),replace[i],backup[i]);
  if(status){
   atomic_store(&enabled,false);int failures=batch_rollback(&batch);
   atomic_store(&failed_hook,(int)i);atomic_store(&hook_error,status);atomic_store(&connection_stage,failures?-4:-3);return false;
  }
 }
 atomic_store(&ready,true);atomic_store(&connection_stage,4);
 __android_log_print(ANDROID_LOG_INFO,"PhigrosLab","157 gameplay controls installed; original storage and network unchanged; autoplay off");return true;
}
static void on_library(const char *name,void *handle){
 if(!name)return;const char *leaf=strrchr(name,'/');leaf=leaf?leaf+1:name;if(strcmp(leaf,"libil2cpp.so"))return;
 pthread_mutex_lock(&attachment_lock);attachment_library(&attachment,handle,install_library);pthread_mutex_unlock(&attachment_lock);
}
__attribute__((visibility("default"),used)) NativeOnModuleLoaded native_init(const NativeAPIEntries *api){
 if(!api||api->version<1||api->version>2||!api->hook_func||!api->unhook_func)return NULL;
 pthread_mutex_lock(&attachment_lock);batch.hook=api->hook_func;batch.unhook=api->unhook_func;atomic_store(&api_received,true);attachment_try(&attachment,install_library);pthread_mutex_unlock(&attachment_lock);
 // Hand the same API to the cloud observers so the panel's sync self-check works.
 cloud_hook_api(api->hook_func,api->unhook_func);
 return on_library;
}
JNIEXPORT jboolean JNICALL Java_com_phigros_lab_LabNative_isReady(JNIEnv *env,jclass type){(void)env;(void)type;return atomic_load(&ready)?JNI_TRUE:JNI_FALSE;}
JNIEXPORT jboolean JNICALL Java_com_phigros_lab_LabNative_readyOrAbort(JNIEnv *env,jclass type){return Java_com_phigros_lab_LabNative_isReady(env,type);}
JNIEXPORT jboolean JNICALL Java_com_phigros_lab_LabNative_setRoot(JNIEnv *env,jclass type,jstring path){
 (void)type;if(!path)return JNI_FALSE;const char *text=(*env)->GetStringUTFChars(env,path,NULL);if(!text)return JNI_FALSE;
 size_t n=strnlen(text,4096);bool valid=n>0&&n<4096&&text[0]=='/'&&!strstr(text,"..")&&!strchr(text,'\n');(*env)->ReleaseStringUTFChars(env,path,text);if(!valid)return JNI_FALSE;
 // Original-data mode never rewrites a cached Unity path. Accept either load order.
 void *existing=dlopen("libil2cpp.so",RTLD_NOW|RTLD_NOLOAD);
 pthread_mutex_lock(&attachment_lock);if(existing)attachment.handle=existing;attachment_prepare(&attachment,install_library);pthread_mutex_unlock(&attachment_lock);if(existing)dlclose(existing);return JNI_TRUE;
}
JNIEXPORT jintArray JNICALL Java_com_phigros_lab_LabNative_diagnostics(JNIEnv *env,jclass type){
 (void)type;pthread_mutex_lock(&attachment_lock);jint values[]={atomic_load(&api_received),attachment.prepared,attachment.handle!=NULL,atomic_load(&connection_stage),atomic_load(&failed_hook),atomic_load(&hook_error)};pthread_mutex_unlock(&attachment_lock);
 jintArray a=(*env)->NewIntArray(env,6);if(a)(*env)->SetIntArrayRegion(env,a,0,6,values);return a;
}
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm,void *reserved){(void)vm;(void)reserved;return JNI_VERSION_1_6;}
