"""CH3 LEDs: share normal tape palette and mark last successful scene recall."""
def tape_led_helper(s):
 marker='static const uint8_t SMK_EFFECT_RGB[6][3]'
 assert marker in s
 helper='''static void ch_tape_leds(void){
 uint32_t i;for(i=0;i<4u;i++){
  uint32_t div=i==ch_sound?1u:16u;const uint8_t *c=SMK_PATCH_RGB[i];
  padled_set(10u+i,c[0]/div,c[1]/div,c[2]/div);
 }
}
'''
 return s.replace(marker,helper+marker)
def scene_leds(s):
 s='static uint32_t sc_loaded=255u; /* last successful recall, not the last saved slot */\n'+s
 a='touched=99;sc_selected=p;';assert a in s;s=s.replace(a,a+'sc_loaded=p;')
 a='  if(i==sc_selected&&filled)v=255;';assert a in s
 s=s.replace(a,'''  uint32_t loaded=i==sc_loaded&&filled;
  if(loaded){uint32_t phase=fm1_ms%1800u,t=phase<900u?phase:1800u-phase;v=160u+t*95u/900u;}''')
 a='  padled_set(i,(uint8_t)(v/3u),(uint8_t)v,(uint8_t)(v*3u/4u));';assert a in s
 return s.replace(a,'''  if(loaded)padled_set(i,v/6u,v,v/3u);
  else padled_set(i,v/3u,v,v*3u/4u);''')
