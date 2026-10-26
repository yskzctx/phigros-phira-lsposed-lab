#ifndef LAB_CONTROLLED_FRAME_H
#define LAB_CONTROLLED_FRAME_H
#include "goal-plan.h"
#include "input-bridge.h"
#include "input-touch.h"
#include "timing-jitter.h"
#include <stdio.h>
/* Pinned original 0.8.2 ABI, verified by original disassembly. This adapter
 * adapts ordinary chart timing/kind/index inputs for one Judge.update and
 * finishes the ordinary note state after a confirmed early Bad.
 * Original input handling, FX/audio and protected score updates still run.
 * Never read, reconstruct or write JudgeInner (Judge+0x58..0x178). */
#define LAB_MAX_LINES 2048
#define LAB_NOTE_BYTES 0x188u
static size_t lab_size(const void *p,size_t offset){size_t v;memcpy(&v,(const char*)p+offset,8);return v;}
static void *lab_ptr(const void *p,size_t offset){void *v;memcpy(&v,(const char*)p+offset,8);return v;}
static double lab_f64(const void *p,size_t offset){double v;memcpy(&v,(const char*)p+offset,8);return v;}
static void lab_put_size(void *p,size_t offset,size_t v){memcpy((char*)p+offset,&v,8);}
static void lab_put_f64(void *p,size_t offset,double v){memcpy((char*)p+offset,&v,8);}
typedef struct {unsigned char *memory;size_t line,id,kind;double start,end,saved_time;size_t saved_kind;bool changed,contact_ready;} LabChartNote;
typedef struct {unsigned char *notes,*live;uint32_t *indices,*order,*rank;size_t count,notes_len,start,end;bool reordered;} LabChartRow;
typedef struct {
 void *judge,*lines,*judge_rows;size_t row_count;int n;uint32_t seed;uint64_t generation;double last_time;
 LabChartNote *notes;LabChartRow *rows;unsigned char *selected,*types;double *timing_offsets;ControlConfig config;PlanEstimate estimate;LabTimingEstimate timing_estimate;
 const LabTouchApi *touch;LabTouchContact *contacts;LabTouchScope touch_scope;bool cleanup_failed;void *frame_resource;uint32_t saved_autoplay;
 LabInputOwner input;bool valid,controlled,in_frame,inspected,expired_miss;unsigned fired;unsigned char first_state;char error[160];
} LabPlan;
static void lab_plan_release(LabPlan *p){
 if(!p)return;
 if(p->rows)for(size_t i=0;i<p->row_count;i++){free(p->rows[i].order);free(p->rows[i].rank);}
 free(p->rows);free(p->notes);free(p->selected);free(p->types);free(p->contacts);free(p->timing_offsets);memset(p,0,sizeof(*p));
}
static int lab_compare_notes(const void *aa,const void *bb){const LabChartNote *a=aa,*b=bb;if(a->start<b->start)return -1;if(a->start>b->start)return 1;if(a->line!=b->line)return a->line<b->line?-1:1;return a->id<b->id?-1:a->id>b->id;}
static int lab_fail(LabPlan *p,const char *message){snprintf(p->error,sizeof(p->error),"%s",message);return 0;}
static bool lab_plan_collect(LabPlan *p,void *judge,void *chart,const ControlConfig *cfg,uint32_t seed,double time,float speed){
 size_t nr=lab_size(judge,16),nc=lab_size(chart,0x80);void *lines=lab_ptr(chart,0x78),*jr=lab_ptr(judge,8);
 if(!nr||nr!=nc||nr>LAB_MAX_LINES||!lines||!jr)return lab_fail(p,"谱面行与判定索引不匹配");
 bool keep_contact=p->valid&&p->judge==judge&&p->lines==lines&&p->judge_rows==jr&&p->input.held;
 if(keep_contact){uint32_t down;memcpy(&down,(char*)judge+0x50,4);keep_contact=down>0;}
 uint64_t generation=p->generation+1;const LabTouchApi *touch=p->touch;
 lab_plan_release(p);p->touch=touch;p->generation=generation;p->judge=judge;p->lines=lines;p->judge_rows=jr;p->row_count=nr;p->seed=seed?seed:123;p->last_time=time;p->input.held=keep_contact;
 p->rows=calloc(nr,sizeof(*p->rows));p->notes=calloc(MAX_NOTES+1,sizeof(*p->notes));p->selected=calloc(MAX_NOTES+1,1);p->types=calloc(MAX_NOTES+1,1);
 p->contacts=calloc(MAX_NOTES+1,sizeof(*p->contacts));
 if(!p->rows||!p->notes||!p->selected||!p->types||!p->contacts)return lab_fail(p,"谱面计划内存不足");
 for(size_t i=0;i<nr;i++){
  LabChartRow *r=p->rows+i;r->live=(unsigned char*)jr+i*32;r->indices=lab_ptr(r->live,8);r->count=lab_size(r->live,16);
  unsigned char *line=(unsigned char*)lines+i*0x300;r->notes=lab_ptr(line,0x1c8);r->notes_len=lab_size(line,0x1d0);
  if(r->count>MAX_NOTES||r->notes_len>MAX_NOTES*4u||r->count>r->notes_len||p->n+(int)r->count>MAX_NOTES||(!r->indices&&r->count)||(!r->notes&&r->notes_len))return lab_fail(p,"谱面索引大小异常或超出支持范围");
  r->order=malloc((r->count?r->count:1)*4);r->rank=calloc(r->notes_len?r->notes_len:1,4);if(!r->order||!r->rank)return lab_fail(p,"谱面索引内存不足");
  if(r->count)memcpy(r->order,r->indices,r->count*4);
  double previous=-INFINITY;
  for(size_t k=0;k<r->count;k++){
   size_t id=r->order[k];if(id>=r->notes_len||r->rank[id])return lab_fail(p,"谱面索引重复或越界");r->rank[id]=1;
   unsigned char *note=r->notes+id*LAB_NOTE_BYTES;size_t kind=lab_size(note,0);double at=lab_f64(note,0x140),end=kind==1?lab_f64(note,8):at;
   if(kind>3||!isfinite(at)||!isfinite(end)||end<at||at<previous)return lab_fail(p,"音符类型、时间或排序异常");
   previous=at;
   p->notes[++p->n]=(LabChartNote){.memory=note,.line=i,.id=id,.kind=kind,.start=at,.end=end};
  }
 }
 if(!p->n)return lab_fail(p,"谱面没有可判定音符");
 qsort(p->notes+1,(size_t)p->n,sizeof(*p->notes),lab_compare_notes);
 PlanTiming *timing=calloc((size_t)p->n+1,sizeof(*timing));if(!timing)return lab_fail(p,"谱面时间计划内存不足");
 for(int i=1;i<=p->n;i++){LabChartNote *n=p->notes+i;p->rows[n->line].rank[n->id]=(uint32_t)i;p->types[i]=(unsigned char)(n->kind==0?1:n->kind==1?3:n->kind==2?4:2);timing[i]=(PlanTiming){(float)n->start,(float)n->end};}
 p->config=*cfg;p->controlled=true;
 PlanTimeline timeline=plan_timeline_windows(timing,p->types,p->n,.12f*speed,.221f*speed,.19f*speed,p->seed);free(timing);
 if(!timeline.events)return lab_fail(p,"谱面判定时间计划无法创建");
 make_profile_timed(cfg,p->n,p->seed,p->selected,&p->estimate,p->types,&timeline);
 p->timing_offsets=calloc((size_t)p->n+1,sizeof(double));if(!p->timing_offsets||!lab_timing_plan(p->selected,p->n,cfg->timing_tenths_ms,p->seed,p->timing_offsets,&p->timing_estimate)){free(timeline.events);return lab_fail(p,"命中误差计划无法创建");}
 if(cfg->timing_tenths_ms>0){for(int i=0;i<timeline.length;i++)if(timeline.events[i].judgment==0){int id=timeline.events[i].note;LabChartNote *note=p->notes+id;double at=note->start+p->timing_offsets[id]*speed;timeline.events[i].time=(float)(note->kind==1?fmax(note->end,at):at);}qsort(timeline.events,(size_t)timeline.length,sizeof(PlanEvent),compare_events);estimate_timed(p->selected,p->n,&p->estimate,&timeline);}
 free(timeline.events);
 p->valid=true;return true;
}
static _Thread_local LabChartRow *lab_sort_row;
static int lab_compare_indices(const void *aa,const void *bb){uint32_t a,b;memcpy(&a,aa,4);memcpy(&b,bb,4);double ta=lab_f64(lab_sort_row->notes+(size_t)a*LAB_NOTE_BYTES,0x140),tb=lab_f64(lab_sort_row->notes+(size_t)b*LAB_NOTE_BYTES,0x140);if(ta<tb)return -1;if(ta>tb)return 1;return a<b?-1:a>b;}
static void lab_frame_end(LabPlan *p){
 if(!p)return;
 p->cleanup_failed=!lab_touch_end(&p->touch_scope);if(p->cleanup_failed)lab_fail(p,"模拟触点未能安全回收，已停止继续输入");
 if(!p->in_frame){if(p->inspected&&p->valid&&p->n)p->first_state=p->notes[1].memory[0x30];p->inspected=false;return;}
 for(int i=1;i<=p->n;i++){LabChartNote *n=p->notes+i;if(n->changed){
  /* The original early Click Bad has already committed and created BadNote,
   * but leaves PreJudge until its ghost cleanup. A restored Drag/Flick kind
   * would later commit a second Perfect. Finish only this confirmed Bad's
   * ordinary note lifecycle before restoring its kind; never touch scores. */
  if(p->selected[i]==3&&n->memory[0x30]==1)n->memory[0x30]=2;
  lab_put_f64(n->memory,0x140,n->saved_time);lab_put_size(n->memory,0,n->saved_kind);n->changed=false;
 }}
 for(size_t i=0;i<p->row_count;i++){
  LabChartRow *r=p->rows+i;if(r->reordered&&r->count)memcpy(r->indices,r->order,r->count*4);r->reordered=false;
  size_t first=0;while(first<r->count&&r->notes[(size_t)r->order[first]*LAB_NOTE_BYTES+0x30]==2)first++;lab_put_size(r->live,24,first);
 }
 if(p->frame_resource){uint32_t mods;memcpy(&mods,(unsigned char*)p->frame_resource+0x88,4);mods=(mods&~1u)|p->saved_autoplay;memcpy((unsigned char*)p->frame_resource+0x88,&mods,4);p->frame_resource=NULL;}
 p->in_frame=false;p->inspected=false;if(p->valid&&p->n)p->first_state=p->notes[1].memory[0x30];
}
static bool lab_frame_begin(LabPlan *p,void *judge,void *resource,void *chart,bool enabled,const ControlConfig *cfg,uint32_t seed,const LabInputApi *api){
 if(!p||!judge||!resource||!chart||!cfg)return false;
 if(!lab_touch_end(&p->touch_scope))return lab_fail(p,"上次模拟触点仍被借用，未继续输入");
 p->cleanup_failed=false;
 p->fired=0;p->expired_miss=false;p->inspected=false;
 if(!enabled){if(p->input.held&&!lab_input_apply(api,&p->input,0,false))return lab_fail(p,"输入释放失败");return true;}
 double time=lab_f64(resource,0x9e0);float speed;memcpy(&speed,(char*)resource+0x9c,4);
 if(!isfinite(time)||!isfinite(speed)||speed<=0||speed>16)return lab_fail(p,"谱面时钟或速度无效");
 size_t nr=lab_size(judge,16);if(nr!=lab_size(chart,0x80)||nr>LAB_MAX_LINES)return lab_fail(p,"谱面行与判定索引不匹配");
 bool same=p->valid&&p->judge==judge&&p->lines==lab_ptr(chart,0x78)&&p->judge_rows==lab_ptr(judge,8)&&p->row_count==nr;
 if(same)for(size_t i=0;i<nr;i++){LabChartRow *r=p->rows+i;unsigned char *line=(unsigned char*)p->lines+i*0x300;if(r->notes!=lab_ptr(line,0x1c8)||r->notes_len!=lab_size(line,0x1d0)||r->indices!=lab_ptr(r->live,8)||r->count!=lab_size(r->live,16)){same=false;break;}}
 if(!same||time<p->last_time-.25||(p->first_state!=0&&p->n&&p->notes[1].memory[0x30]==0)){if(!lab_plan_collect(p,judge,chart,cfg,seed,time,speed))return false;}
 p->last_time=time;p->inspected=true;
 for(int i=1;i<=p->n;i++)if(lab_size(p->notes[i].memory,0)!=p->notes[i].kind||lab_f64(p->notes[i].memory,0x140)!=p->notes[i].start){p->inspected=false;return lab_fail(p,"谱面运行时结构已改变，未继续自动输入");}
 uint32_t mods;memcpy(&mods,(char*)resource+0x88,4);uint32_t manual=mods&~1u;memcpy((char*)resource+0x88,&manual,4);
 unsigned char old_keyboard=((unsigned char*)resource)[0xbc];if(!p->touch)((unsigned char*)resource)[0xbc]=1;p->frame_resource=resource;p->saved_autoplay=mods&1u;p->in_frame=true;
 for(int i=1;i<=p->n;i++){
  LabChartNote *n=p->notes+i;n->contact_ready=false;unsigned char state=n->memory[0x30];if(state!=0)continue;int judgment=p->selected[i];
  /* Original touch matching subtracts EARLY_OFFSET=.07 from late offsets.
   * +.12 would become .05 (Perfect); +.19 becomes .12 (Good). */
  double good_offset=plan_good_early(judgment,i,p->seed)?-.12:.19;
  double perfect_offset=p->timing_offsets[i];double event=n->start;if(judgment==1)event+=.221*speed;else if(judgment==3)event-=.19*speed;else if(plan_is_good(judgment))event+=good_offset*speed;else event+=perfect_offset*speed;
  bool ready=time>=event;if(judgment==1&&ready)p->expired_miss=true;
  if(n->start>time+.25*speed&&!ready)continue;
  n->saved_time=lab_f64(n->memory,0x140);n->saved_kind=lab_size(n->memory,0);n->changed=true;
  if(judgment!=1&&(n->kind!=1||judgment==3))lab_put_size(n->memory,0,0); // ordinary chart input classification; never a score-field write
  if(!ready)lab_put_f64(n->memory,0x140,time+.221*speed);
  else if(judgment!=1){double diff=judgment==3?-.19:plan_is_good(judgment)?good_offset:perfect_offset;lab_put_f64(n->memory,0x140,time-diff*speed);n->contact_ready=true;p->fired++;}
 }
 for(size_t i=0;i<p->row_count;i++){
  LabChartRow *r=p->rows+i;r->start=lab_size(r->live,24);if(r->start>r->count){lab_frame_end(p);memcpy((char*)resource+0x88,&mods,4);((unsigned char*)resource)[0xbc]=old_keyboard;return lab_fail(p,"谱面判定游标越界");}
  r->end=r->start;while(r->end<r->count){size_t rank=r->rank[r->order[r->end]];if(p->notes[rank].start>time+.25*speed)break;r->end++;}
  if(r->end>r->start){lab_sort_row=r;qsort(r->indices+r->start,r->end-r->start,4,lab_compare_indices);r->reordered=true;}
 }
 if(p->expired_miss)p->fired=0;
 bool submitted;
 if(p->touch){size_t count=0;for(int i=1;i<=p->n;i++){LabChartNote *n=p->notes+i;if((n->kind==1&&n->memory[0x30]==3)||(!p->expired_miss&&n->contact_ready))p->contacts[count++]=(LabTouchContact){.line=(unsigned char*)p->lines+n->line*0x300,.note=n->memory,.id=0x5048495241000000ULL+(unsigned)i};}submitted=lab_touch_submit_scoped(p->touch,p->contacts,count,resource,p->lines,p->row_count,time,&p->touch_scope);}
 else submitted=lab_input_apply(api,&p->input,p->fired,true);
 if(!submitted){lab_frame_end(p);memcpy((char*)resource+0x88,&mods,4);((unsigned char*)resource)[0xbc]=old_keyboard;return lab_fail(p,"判定输入接口忙、坐标无效或未就绪");}
 p->error[0]=0;return true;
}
#endif
