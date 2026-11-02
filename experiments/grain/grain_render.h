/* SPDX-License-Identifier: GPL-3.0-only */
static void gr_render(uint32_t vi,int32_t *acc,int32_t c16){
 uint32_t i,k;int32_t wet[SUB]={0},tmp[SUB];
 int32_t old=gr_voice[vi].mix,target=(int32_t)gr_ctl[3]*32767/100,next=old;
 if(next<target){next+=1024;if(next>target)next=target;}
 if(next>target){next-=1024;if(next<target)next=target;}
 gr_voice[vi].mix=next;
 if(!old&&!next){gr_voice[vi].wait=0;gr_voice[vi].g[0].len=gr_voice[vi].g[1].len=0;return;}
 gr_frames+=SUB;
 if(!gr_voice[vi].wait){
  gr_head_t *g=0;
  for(k=0;k<2u;k++)if(!gr_voice[vi].g[k].len){g=&gr_voice[vi].g[k];break;}
  if(g){
   gr_seek(&g->a,&voice[vi].h);g->layer=voice[vi].layer!=0;
   if(g->layer)gr_seek(&g->b,&voice[vi].h2);
   g->age=0;g->len=(gr_ctl[0]*FS/1000u/SUB)*SUB;
   if(g->len<2u*SUB)g->len=2u*SUB;
   g->phase=0;g->inc=(32767u<<16)/g->len;
   gr_spawns++;
  }
  /* DENSITY 100: two overlapping grains; lower values create soft gaps.
   * Quantise launch interval upwards so the two-slot bound is always respected. */
  uint32_t hop=(gr_ctl[0]*FS/1000u)*50u/gr_ctl[2];
  gr_voice[vi].wait=((hop+SUB-1u)/SUB)*SUB;
 }
 gr_voice[vi].wait=gr_voice[vi].wait>SUB?gr_voice[vi].wait-SUB:0;
 for(k=0;k<2u;k++){
  gr_head_t *g=&gr_voice[vi].g[k];if(!g->len)continue;
  for(i=0;i<SUB;i++)tmp[i]=0;
  uint32_t note=voice[vi].note;
  head_run(&g->a,step_of(((int32_t)note-g->a.z->root)*1600+c16),tmp,SUB,g->layer?p_ga:32767,g->a.pos);
  if(g->layer)head_run(&g->b,step_of(((int32_t)layer_note(note,voice[vi].loct)-g->b.z->root)*1600+c16+p_ldet16),tmp,SUB,p_gb,g->b.pos);
  /* Parabolic zero-ended window, constant overlap close to unity at density 100.
   * Values stay <=32767 and all products fit signed 32-bit. */
  for(i=0;i<SUB;i++){
   uint32_t x=g->phase>>16;g->phase+=g->inc;
   int32_t w=(int32_t)((x*(32767u-x))>>13); /* 0..32767 */
   /* A second smooth taper reduces high-frequency grain edges. */
   w=(w*w)>>15;
   wet[i]+=(tmp[i]*w)>>15;
  }
  g->age+=SUB;if(g->age>=g->len)g->len=0;
 }
 for(i=0;i<SUB;i++){
  int32_t m=old+(next-old)*(int32_t)i/(int32_t)SUB;
  /* 0.8 wet headroom for correlated overlap, shared downstream FX unchanged. */
  int32_t w=(wet[i]*26214)>>15;
  acc[i]=(acc[i]*(32767-m)>>15)+(w*m>>15);
 }
}
