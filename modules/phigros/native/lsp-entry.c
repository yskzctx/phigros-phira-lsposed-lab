// Runtime attachment only. Never modifies the installed game's APK or signature.
#define JNI_OnLoad offline_unused_onload
#include "core/offline.c"
#undef JNI_OnLoad
#define MODULE_HOST_TEST
#include "cloud-guard.c"
#undef MODULE_HOST_TEST
#include "cloud-profile.h"
#include "persistent-profile.h"
#include "../../common/hook-batch.h"
#include "../../common/activation-gate.h"
#include <dlfcn.h>

typedef void (*NativeOnModuleLoaded)(const char*,void*);
typedef struct{uint32_t version;LabHook hook_func;LabUnhook unhook_func;} NativeAPIEntries;
static LabBatch batch;
static LabGate gate=LAB_GATE_INITIALIZER;
static atomic_bool root_ready=false;
static char lab_files[512];
static void *(*persistent_original)(const void*);
static void *(*managed_string)(const char*);
static void *persistent_path(const void *method){
 if(atomic_load(&isolated)&&managed_string)return managed_string(lab_files);
 return persistent_original(method);
}
typedef struct{uintptr_t base;bool okay[12];} Ranges;
static int ranges(struct dl_phdr_info *info,size_t size,void *data){
 (void)size;Ranges *r=data;if(info->dlpi_addr!=r->base)return 0;
 for(size_t k=0;k<12;k++){
  uintptr_t target=k<9?hooks[k].rva:k<11?cloud_hooks[k-9].rva:persistent_hook.rva;
  for(size_t i=0;i<info->dlpi_phnum;i++){const ElfW(Phdr)*p=&info->dlpi_phdr[i];
   if(p->p_type==PT_LOAD&&(p->p_flags&PF_X)&&target>=p->p_vaddr&&target-p->p_vaddr<=p->p_memsz&&p->p_memsz-(target-p->p_vaddr)>=16)r->okay[k]=true;
  }
 }return 1;
}
static void install_library(void *handle){
 if(!handle||!batch.hook||atomic_load(&ready))return;
 Dl_info location;void *symbol=dlsym(handle,"il2cpp_domain_get");if(!symbol||!dladdr(symbol,&location)||!location.dli_fbase)return;
 image=(uintptr_t)location.dli_fbase;Ranges r={.base=image};dl_iterate_phdr(ranges,&r);
 for(size_t i=0;i<12;i++){
  uintptr_t at=i<9?hooks[i].rva:i<11?cloud_hooks[i-9].rva:persistent_hook.rva;
  const unsigned char *prefix=i<9?hooks[i].prefix:i<11?cloud_hooks[i-9].prefix:persistent_hook.prefix;
  if(!r.okay[i]||memcmp((void*)(image+at),prefix,16)){__android_log_print(ANDROID_LOG_ERROR,"PhigrosLab","Unsupported native profile; lab remains inactive");return;}
 }
 managed_string=(void*(*)(const char*))dlsym(handle,"il2cpp_string_new");if(!managed_string)return;
 bad_call=(void(*)(void*,float,float,const void*))(image+0x1d875c4);
 void *replace[]={(void*)click,(void*)drag,(void*)flick,(void*)hold,(void*)record,(void*)perfect,(void*)result,(void*)miss,(void*)good,(void*)guard_sync,(void*)guard_silent,(void*)persistent_path};
 void **backup[]={(void**)&click_original,(void**)&drag_original,(void**)&flick_original,(void**)&hold_original,(void**)&record_original,(void**)&perfect_original,(void**)&result_original,(void**)&miss_original,(void**)&good_call,(void**)&sync_original,(void**)&silent_original,(void**)&persistent_original};
 for(size_t i=0;i<12;i++){
  uintptr_t at=i<9?hooks[i].rva:i<11?cloud_hooks[i-9].rva:persistent_hook.rva;
  if(batch_install(&batch,(void*)(image+at),replace[i],backup[i])){
   atomic_store(&enabled,false);int failures=batch_rollback(&batch);
   __android_log_print(ANDROID_LOG_ERROR,"PhigrosLab","Lab install incomplete, rollback failures=%d; no storage activation",failures);return;
  }
 }
 atomic_store(&guard_ready,true);activate_isolation(true);atomic_store(&ready,true);gate.ready=true;
 __android_log_print(ANDROID_LOG_INFO,"PhigrosLab","157 control, local save isolation and upload guard installed; autoplay off");
}
static void on_library(const char *name,void *handle){
 if(!name)return;const char *leaf=strrchr(name,'/');leaf=leaf?leaf+1:name;if(strcmp(leaf,"libil2cpp.so"))return;
 pthread_mutex_lock(&gate.lock);if(atomic_load(&root_ready)&&!gate.aborted)install_library(handle);pthread_mutex_unlock(&gate.lock);
}
__attribute__((visibility("default"),used)) NativeOnModuleLoaded native_init(const NativeAPIEntries *api){
 if(!api||api->version<1||api->version>2||!api->hook_func||!api->unhook_func)return NULL;
 batch.hook=api->hook_func;batch.unhook=api->unhook_func;return on_library;
}
JNIEXPORT jboolean JNICALL Java_com_phigros_lab_LabNative_isReady(JNIEnv *env,jclass type){(void)env;(void)type;return atomic_load(&ready)&&atomic_load(&isolated)?JNI_TRUE:JNI_FALSE;}
JNIEXPORT jboolean JNICALL Java_com_phigros_lab_LabNative_readyOrAbort(JNIEnv *env,jclass type){(void)env;(void)type;return gate_ready_or_abort(&gate)?JNI_TRUE:JNI_FALSE;}
JNIEXPORT jboolean JNICALL Java_com_phigros_lab_LabNative_setRoot(JNIEnv *env,jclass type,jstring path){
 (void)type;if(!path||atomic_load(&root_ready))return JNI_FALSE;const char *text=(*env)->GetStringUTFChars(env,path,NULL);if(!text)return JNI_FALSE;
 size_t n=strnlen(text,sizeof(lab_files));bool valid=n>0&&n<sizeof(lab_files)&&text[0]=='/'&&!strstr(text,"..")&&!strchr(text,'\n');
 pthread_mutex_lock(&gate.lock);dl_iterate_phdr(find_image,NULL);
 if(gate.aborted||image)valid=false; // Refuse late attachment after Unity may have cached the original save.
 if(valid){memcpy(lab_files,text,n+1);atomic_store(&root_ready,true);}else gate.aborted=true;
 pthread_mutex_unlock(&gate.lock);(*env)->ReleaseStringUTFChars(env,path,text);return valid?JNI_TRUE:JNI_FALSE;
}
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm,void *reserved){(void)vm;(void)reserved;return JNI_VERSION_1_6;}
