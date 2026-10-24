#define _GNU_SOURCE
#include <jni.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <link.h>
#include <pthread.h>
#include <time.h>
#include <android/log.h>
#include "controlled-frame.h"
#include "profile.h"
#include "../../common/hook-batch.h"
typedef void(*LibraryLoaded)(const char*,void*);
typedef struct{uint32_t version;LabHook hook;LabUnhook unhook;} NativeAPI;
static LabBatch batch;static pthread_mutex_t lock=PTHREAD_MUTEX_INITIALIZER;
static atomic_bool ready,armed,enabled,tainted,early_frame;
static atomic_uint frames;
/* 0 callback absent, 1 waiting, 2 symbol missing, 3 profile mismatch,
 * 4 hook failure, 5 installed. Never reports active from Java scope alone. */
static atomic_int stage;
static pthread_mutex_t config_lock=PTHREAD_MUTEX_INITIALIZER,status_lock=PTHREAD_MUTEX_INITIALIZER;
static ControlConfig pending_config={.target=1000000,.accuracy=10000},frame_config;
static atomic_uint config_revision;
static LabPlan plan;
static LabInputApi input_api;
static LabTouchApi touch_api;static void *anim_value_original;
extern float lab_anim_value(const void*,void*);
static float note_x_value(const void *animation){return lab_anim_value(animation,anim_value_original);}
static double input_uptime(void){struct timespec ts;if(clock_gettime(CLOCK_MONOTONIC,&ts))return NAN;return (double)ts.tv_sec+(double)ts.tv_nsec*1e-9;}
typedef struct {int counts[4],combo,max_combo,early,late;uint64_t generation;} Observed;
static Observed observed;
static _Thread_local void *observed_judge;
static jint controls_status[21];static char controls_detail[192];
static void (*judge_original)(void*,void*,void*,void*);
static void (*commit_original)(void*,unsigned,double);
/* Observation only: forward every original parameter unchanged and let the
 * original protected update run. Never reconstruct/patch protected values. */
static void commit_observer(void *inner,unsigned what,double diff){
 commit_original(inner,what,diff);
 if(!observed_judge||inner!=(unsigned char*)observed_judge+0x58||what>3)return;
 observed.counts[what]++;
 if(what<2){observed.combo++;if(observed.combo>observed.max_combo)observed.max_combo=observed.combo;}else observed.combo=0;
 if(what==1){if(diff<0)observed.early++;else observed.late++;}
}
static void publish_controls(void){
 int n=plan.valid?plan.n:0,total=0;for(int i=0;i<4;i++)total+=observed.counts[i];
 double accuracy=n?((double)observed.counts[0]+.65*observed.counts[1])/n:0;
 int score=n?(int)floor((.9*accuracy+.1*(double)observed.max_combo/n)*1000000.+.5):0;
 PlanEstimate *e=&plan.estimate;
 jint v[]={atomic_load(&ready),atomic_load(&enabled),n,e->good+e->bad+e->miss,observed.counts[1]+observed.counts[2]+observed.counts[3],(jint)atomic_load(&config_revision),e->score,e->accuracy,e->miss,e->good,e->bad,n>0&&total>=n,score,(int)floor(accuracy*10000.+.5),observed.counts[0],observed.counts[1],observed.counts[2],observed.counts[3],observed.max_combo,observed.early,observed.late};
 pthread_mutex_lock(&status_lock);memcpy(controls_status,v,sizeof(v));snprintf(controls_detail,sizeof(controls_detail),"%s",plan.error);pthread_mutex_unlock(&status_lock);
}
static void judge_update(void *judge,void *resource,void *chart,void *bad_notes){
 if(!atomic_load(&armed)){judge_original(judge,resource,chart,bad_notes);return;}
 plan.touch=&touch_api;
 pthread_mutex_lock(&config_lock);frame_config=pending_config;pthread_mutex_unlock(&config_lock);
 bool requested=atomic_load(&enabled);bool okay=lab_frame_begin(&plan,judge,resource,chart,requested,&frame_config,(uint32_t)((uintptr_t)chart^(uintptr_t)judge^0xa73c819du),&input_api);
 if(!okay)atomic_store(&enabled,false);
 if(observed.generation!=plan.generation){memset(&observed,0,sizeof(observed));observed.generation=plan.generation;}
 void *previous=observed_judge;observed_judge=okay&&requested?judge:NULL;
 judge_original(judge,resource,chart,bad_notes);
 observed_judge=previous;lab_frame_end(&plan);publish_controls();
}
static uintptr_t (*game_original)(void*,void*);
static uintptr_t game_update(void *scene,void *time_manager){
 pthread_mutex_lock(&lock);bool active=atomic_load(&armed);if(!active)atomic_store(&early_frame,true);pthread_mutex_unlock(&lock);
 if(!active)return game_original(scene,time_manager);
 bool autoplay=atomic_load(&enabled);if(autoplay)atomic_store(&tainted,true);
 atomic_fetch_add(&frames,1);
 return game_original(scene,time_manager);
}
typedef struct{uintptr_t base;bool okay[LAB_PROFILE_COUNT],key;} Range;
static int range(struct dl_phdr_info *info,size_t unused,void *data){
 (void)unused;Range *r=data;if(info->dlpi_addr!=r->base)return 0;
 for(size_t i=0;i<info->dlpi_phnum;i++){const ElfW(Phdr)*p=&info->dlpi_phdr[i];if(p->p_type!=PT_LOAD)continue;
  for(size_t k=0;k<LAB_PROFILE_COUNT;k++)if((p->p_flags&PF_X)&&lab_function_rvas[k]>=p->p_vaddr&&lab_function_rvas[k]-p->p_vaddr<=p->p_memsz&&p->p_memsz-(lab_function_rvas[k]-p->p_vaddr)>=16)r->okay[k]=true;
  if((p->p_flags&PF_R)&&INPUT_LOCALKEY_RVA>=p->p_vaddr&&INPUT_LOCALKEY_RVA-p->p_vaddr<=p->p_memsz&&p->p_memsz-(INPUT_LOCALKEY_RVA-p->p_vaddr)>=8)r->key=true;
 }return 1;
}
static void loaded(const char *name,void *handle){
 if(!name||!handle)return;const char *leaf=strrchr(name,'/');leaf=leaf?leaf+1:name;if(strcmp(leaf,"libphira.so"))return;
 pthread_mutex_lock(&lock);if(atomic_load(&ready)||!batch.hook){pthread_mutex_unlock(&lock);return;}
 void *symbol=dlsym(handle,"Java_quad_1native_QuadNative_setDataPath");Dl_info info;
 if(!symbol||!dladdr(symbol,&info)||!info.dli_fbase){atomic_store(&stage,2);pthread_mutex_unlock(&lock);return;}
 Range r={.base=(uintptr_t)info.dli_fbase};dl_iterate_phdr(range,&r);
 bool valid=r.key;
 for(size_t k=0;k<LAB_PROFILE_COUNT;k++)if(!r.okay[k]||memcmp((void*)(r.base+lab_function_rvas[k]),lab_function_prefixes[k],16))valid=false;
 uintptr_t key_function=0;if(valid)memcpy(&key_function,(void*)(r.base+INPUT_LOCALKEY_RVA),8);if(key_function!=r.base+INPUT_THUNK_RVA)valid=false;
 if(!valid){atomic_store(&stage,3);__android_log_print(ANDROID_LOG_ERROR,"PhiraLab","Input/native profile mismatch; no activation");pthread_mutex_unlock(&lock);return;}
 input_api.local_key=(void*(*)(void*))key_function;input_api.repeat=(void(*)(const void*,void*,void*))(r.base+INPUT_REPEAT_RVA);
 anim_value_original=(void*)(r.base+ANIM_VALUE_RVA);
 touch_api=(LabTouchApi){.input=input_api,.set_time=(void(*)(void*,double))(r.base+OBJECT_TIME_RVA),.line_position=(uint64_t(*)(const void*,const void*,const void*,size_t))(r.base+LINE_POSITION_RVA),.line_rotation=(float(*)(const void*,const void*,size_t))(r.base+LINE_ROTATION_RVA),.note_x=note_x_value,.screen_width=(float(*)(void))(r.base+SCREEN_WIDTH_RVA),.screen_height=(float(*)(void))(r.base+SCREEN_HEIGHT_RVA),.uptime=input_uptime};
 void *replacements[]={(void*)game_update,(void*)judge_update,(void*)commit_observer};void **backups[]={(void**)&game_original,(void**)&judge_original,(void**)&commit_original};uintptr_t hook_entries[]={GAME_UPDATE_RVA,JUDGE_UPDATE_RVA,JUDGE_COMMIT_RVA};bool installed=true;
 for(size_t k=0;k<3;k++)if(batch_install(&batch,(void*)(r.base+hook_entries[k]),replacements[k],backups[k])){installed=false;break;}
 if(installed&&game_original&&judge_original&&commit_original){atomic_store(&stage,5);atomic_store(&ready,true);__android_log_print(ANDROID_LOG_INFO,"PhiraLab","Original judging/input adapter installed; native upload eligibility preserved; autoplay off");}else{atomic_store(&stage,4);batch_rollback(&batch);}
 pthread_mutex_unlock(&lock);
}
__attribute__((visibility("default"),used)) LibraryLoaded native_init(const NativeAPI *api){if(!api||api->version<1||api->version>2||!api->hook||!api->unhook)return NULL;batch.hook=api->hook;batch.unhook=api->unhook;atomic_store(&stage,1);void *existing=dlopen("libphira.so",RTLD_NOW|RTLD_NOLOAD);if(existing){loaded("libphira.so",existing);dlclose(existing);}return loaded;}
JNIEXPORT jboolean JNICALL Java_com_phira_lab_LabNative_arm(JNIEnv *env,jclass type){(void)env;(void)type;pthread_mutex_lock(&lock);bool okay=atomic_load(&ready)&&!atomic_load(&early_frame);if(okay)atomic_store(&armed,true);pthread_mutex_unlock(&lock);return okay?JNI_TRUE:JNI_FALSE;}
JNIEXPORT void JNICALL Java_com_phira_lab_LabNative_disarm(JNIEnv *env,jclass type){(void)env;(void)type;atomic_store(&enabled,false);atomic_store(&armed,false);}
JNIEXPORT jboolean JNICALL Java_com_phira_lab_LabNative_setEnabled(JNIEnv *env,jclass type,jboolean value){(void)env;(void)type;if(!atomic_load(&ready)||!atomic_load(&armed))return JNI_FALSE;atomic_store(&enabled,value==JNI_TRUE);return JNI_TRUE;}
JNIEXPORT jboolean JNICALL Java_com_phira_lab_LabNative_setTemporaryRoot(JNIEnv *env,jclass type,jstring directory){(void)type;if(!directory||!atomic_load(&armed))return JNI_FALSE;const char *p=(*env)->GetStringUTFChars(env,directory,NULL);if(!p)return JNI_FALSE;size_t n=strnlen(p,1024);bool okay=n>0&&n<1024&&p[0]=='/'&&!strstr(p,"..")&&!strchr(p,'\n');if(okay)okay=setenv("TMPDIR",p,1)==0;(*env)->ReleaseStringUTFChars(env,directory,p);return okay?JNI_TRUE:JNI_FALSE;}
JNIEXPORT jintArray JNICALL Java_com_phira_lab_LabNative_status(JNIEnv *env,jclass type){(void)type;jint v[]={atomic_load(&ready),atomic_load(&armed),atomic_load(&enabled),atomic_load(&tainted),(jint)atomic_load(&frames),atomic_load(&stage),atomic_load(&early_frame)};jintArray out=(*env)->NewIntArray(env,7);if(out)(*env)->SetIntArrayRegion(env,out,0,7,v);return out;}
JNIEXPORT jboolean JNICALL Java_com_phira_lab_LabNative_configure(JNIEnv *env,jclass type,jint mode,jint target,jint accuracy,jintArray words){
 (void)type;if(mode<0||mode>2||target<0||target>1000000||accuracy<0||accuracy>10000||!words)return JNI_FALSE;
 int length=(*env)->GetArrayLength(env,words);if(length<1||length>MAX_CONFIG_WORDS)return JNI_FALSE;
 jint packed[MAX_CONFIG_WORDS];(*env)->GetIntArrayRegion(env,words,0,length,packed);if((*env)->ExceptionCheck(env))return JNI_FALSE;
 bool typed=packed[0]==-168;if(typed&&length<2)return JNI_FALSE;
 ControlConfig c={.mode=mode,.target=target,.accuracy=accuracy,.rule_count=packed[typed?1:0]};if(c.rule_count<0||c.rule_count>MAX_RULES)return JNI_FALSE;int cursor=typed?2:1;
 for(int i=0;i<c.rule_count;i++){
  if(cursor+(typed?7:6)>length)return JNI_FALSE;
  NoteRule *r=&c.rules[i];r->judgment=packed[cursor++];r->position=packed[cursor++];r->start=packed[cursor++];r->end=packed[cursor++];r->count=packed[cursor++];if(typed)r->note_type=packed[cursor++];r->length=packed[cursor++];
  if(r->note_type<0||r->note_type>4||r->judgment<0||r->judgment>5||r->position<1||r->position>6||r->start<1||r->start>MAX_NOTES||r->end<1||r->end>MAX_NOTES||r->count<1||r->count>MAX_NOTES||r->length<0||r->length>MAX_INDICES||cursor+r->length>length||(r->position==2&&r->end<r->start)||(r->position==3&&r->length==0))return JNI_FALSE;
  for(int k=0;k<r->length;k++){int v=packed[cursor++];if(v<1||v>MAX_NOTES)return JNI_FALSE;r->indices[k]=v;}
 }
 if(cursor!=length)return JNI_FALSE;
 pthread_mutex_lock(&config_lock);pending_config=c;pthread_mutex_unlock(&config_lock);atomic_fetch_add(&config_revision,1);return JNI_TRUE;
}
JNIEXPORT jintArray JNICALL Java_com_phira_lab_LabNative_controlStatus(JNIEnv *env,jclass type){
 (void)type;jint v[21];pthread_mutex_lock(&status_lock);memcpy(v,controls_status,sizeof(v));pthread_mutex_unlock(&status_lock);v[0]=atomic_load(&ready);v[1]=atomic_load(&enabled);v[5]=(jint)atomic_load(&config_revision);
 jintArray out=(*env)->NewIntArray(env,21);if(out)(*env)->SetIntArrayRegion(env,out,0,21,v);return out;
}
JNIEXPORT jstring JNICALL Java_com_phira_lab_LabNative_controlDetail(JNIEnv *env,jclass type){
 (void)type;char detail[192];pthread_mutex_lock(&status_lock);memcpy(detail,controls_detail,sizeof(detail));pthread_mutex_unlock(&status_lock);detail[sizeof(detail)-1]=0;return (*env)->NewStringUTF(env,detail);
}
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm,void *unused){(void)vm;(void)unused;return JNI_VERSION_1_6;}
