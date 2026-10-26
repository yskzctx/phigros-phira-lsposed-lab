#include <assert.h>
#include <stdio.h>
#include "../native/timing-jitter.h"
int main(void){
 unsigned char selected[20001]={0};double offsets[20001];LabTimingEstimate estimate;
 for(int n=1;n<=20000;n=n<10?n+1:n*10){
  for(int t=0;t<=750;t+=25){assert(lab_timing_plan(selected,n,t,123,offsets,&estimate));double square=0;for(int i=1;i<=n;i++){assert(isfinite(offsets[i])&&fabs(offsets[i])<=.07500000001);square+=offsets[i]*offsets[i];}assert(fabs(sqrt(square/n)-t/10000.)<1e-9);}
 }
 assert(lab_timing_plan(selected,20000,200,55,offsets,&estimate));int early=0,late=0;for(int i=1;i<=20000;i++){early+=offsets[i]<0;late+=offsets[i]>0;}assert(early>8000&&late>8000);
 selected[1]=1;selected[2]=4;selected[3]=5;selected[4]=3;assert(lab_timing_plan(selected,4,200,123,offsets,&estimate));assert(!estimate.reachable&&estimate.expected>.02);for(int i=1;i<=4;i++)assert(offsets[i]==0);
 assert(!lab_timing_plan(selected,4,-1,1,offsets,&estimate));assert(!lab_timing_plan(selected,4,751,1,offsets,&estimate));
 puts("PASS: RMS goals, balanced early/late, bounded Perfect offsets, rule priority, empty Perfect sets and invalid bounds");
}
