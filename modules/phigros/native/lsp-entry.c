// Runtime attachment only. Never modifies the installed game's APK or signature.
#define JNI_OnLoad offline_unused_onload
#include "core/offline.c"
#undef JNI_OnLoad
#include "../../common/hook-batch.h"
#include "attachment-order.h"
#include <dlfcn.h>

typedef void (*NativeOnModuleLoaded)(const char*,void*);
typedef struct{uint32_t version;LabHook hook_func;LabUnhook unhook_func;} NativeAPIEntries;
static LabBatch batch;
static AttachOrder attachment;
static pthread_mutex_t attachment_lock=PTHREAD_MUTEX_INITIALIZER;
static atomic_int connection_stage,failed_hook=-1,hook_error;
static atomic_bool api_received;
typedef struct{uintptr_t base;bool okay[9];} Ranges;
static int ranges(struct dl_phdr_info *info,size_t size,void *data){
 (void)size;Ranges *r=data;if(info->dlpi_addr!=r->base)return 0;
 for(size_t k=0;k<9;k++){
  uintptr_t target=hooks[k].rva;
  for(size_t i=0;i<info->dlpi_phnum;i++){const ElfW(Phdr)*p=&info->dlpi_phdr[i];
   if(p->p_type==PT_LOAD&&(p->p_flags&PF_X)&&target>=p->p_vaddr&&target-p->p_vaddr<=p->p_memsz&&p->p_memsz-(target-p->p_vaddr)>=16)r->okay[k]=true;
  }
 }return 1;
}
static bool install_library(void *handle){
 if(atomic_load(&ready))return true;
 if(!handle||!batch.hook||!batch.unhook||atomic_load(&connection_stage)<0)return false;
 atomic_store(&connection_stage,3);
 Dl_info location;void *symbol=dlsym(handle,"il2cpp_domain_get");if(!symbol||!dladdr(symbol,&location)||!location.dli_fbase){atomic_store(&connection_stage,-1);return false;}
 image=(uintptr_t)location.dli_fbase;Ranges r={.base=image};dl_iterate_phdr(ranges,&r);
 for(size_t i=0;i<9;i++){
  uintptr_t at=hooks[i].rva;
  const unsigned char *prefix=hooks[i].prefix;
  if(!r.okay[i]||memcmp((void*)(image+at),prefix,16)){atomic_store(&failed_hook,(int)i);atomic_store(&connection_stage,-2);return false;}
 }
 bad_call=(void(*)(void*,float,float,const void*))(image+0x1d875c4);
 void *replace[]={(void*)click,(void*)drag,(void*)flick,(void*)hold,(void*)record,(void*)perfect,(void*)result,(void*)miss,(void*)good};
 void **backup[]={(void**)&click_original,(void**)&drag_original,(void**)&flick_original,(void**)&hold_original,(void**)&record_original,(void**)&perfect_original,(void**)&result_original,(void**)&miss_original,(void**)&good_call};
 for(size_t i=0;i<9;i++){
  uintptr_t at=hooks[i].rva;int status=batch_install(&batch,(void*)(image+at),replace[i],backup[i]);
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
 pthread_mutex_lock(&attachment_lock);batch.hook=api->hook_func;batch.unhook=api->unhook_func;atomic_store(&api_received,true);attachment_try(&attachment,install_library);pthread_mutex_unlock(&attachment_lock);return on_library;
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
