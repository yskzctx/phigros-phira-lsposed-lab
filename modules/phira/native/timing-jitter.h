#ifndef LAB_TIMING_JITTER_H
#define LAB_TIMING_JITTER_H
#include "goal-plan.h"
#include <stdbool.h>
typedef struct {double expected;bool reachable;} LabTimingEstimate;
static uint32_t lab_timing_random(uint32_t *s){*s^=*s<<13;*s^=*s>>17;*s^=*s<<5;return *s;}
/* Ordinary input offsets only. No result-field or protected JudgeInner writes.
 * Closed0.8.2 accumulates squared diffs, including Miss=.25, and displays RMS. */
static bool lab_timing_plan(const unsigned char *selected,int n,int tenths_ms,uint32_t seed,double *offsets,LabTimingEstimate *estimate){
 if(!selected||!offsets||!estimate||n<1||n>MAX_NOTES||tenths_ms<0||tenths_ms>750)return false;
 memset(offsets,0,((size_t)n+1)*sizeof(*offsets));int perfect=0;double fixed=0;
 for(int i=1;i<=n;i++){int j=selected[i];if(j==0){perfect++;continue;}double d=j==1?.25:j==3?-.19:plan_good_early(j,i,seed)?-.12:.19;fixed+=d*d;}
 double requested=tenths_ms/10000.;double remaining=requested*requested*n-fixed;
 if(tenths_ms==0||perfect==0||remaining<=0){estimate->expected=sqrt(fixed/n);estimate->reachable=tenths_ms==0||fabs(estimate->expected-requested)<1e-8;return true;}
 double desired=fmin(remaining,perfect*.075*.075);estimate->reachable=remaining<=perfect*.075*.075+1e-12;
 uint32_t state=seed?seed:0x9e3779b9u;int seen=0,first_sign=(lab_timing_random(&state)&1)?1:-1;
 for(int i=1;i<=n;i++)if(selected[i]==0){double weight=.1+.9*(lab_timing_random(&state)/(double)UINT32_MAX);int sign=seen<2?(seen? -first_sign:first_sign):((lab_timing_random(&state)&1)?1:-1);offsets[i]=weight*sign;seen++;}
 double low=0,high=1;
 for(int k=0;k<64;k++){double factor=(low+high)/2,square=0;for(int i=1;i<=n;i++)if(selected[i]==0){double d=fmin(fabs(offsets[i])*factor,.075);square+=d*d;}if(square<desired)low=factor;else high=factor;}
 double square=fixed;for(int i=1;i<=n;i++)if(selected[i]==0){offsets[i]=copysign(fmin(fabs(offsets[i])*high,.075),offsets[i]);square+=offsets[i]*offsets[i];}
 estimate->expected=sqrt(square/n);return true;
}
#endif
