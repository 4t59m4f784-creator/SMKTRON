#if defined(DINGHY_WEB)
EXPORT uint32_t web_scene_loaded(void){return sc_loaded;}
EXPORT void web_chord_mixer(uint32_t k,int32_t value){if(k<3u)ch_mixer(k,value);}
EXPORT void web_chord_mode(uint32_t on){ch_mode(on);}
EXPORT uint8_t *web_chord_flash(void){ch_store_io_init();return ch_store_flash;}
EXPORT uint32_t web_chord_writes(void){return ch_store_store_writes;}
EXPORT int32_t web_chord_state(uint32_t i){switch(i){case 0:return ch_on;case 1:return ch_hold;case 2:return ch_rec;case 3:return ch_target;case 4:return ch_active;case 5:return ch_sound;case 6:return ch_n;case 7:return ch_mix;case 8:return ch_pan_m;case 9:return ch_pan_c;default:return 0;}}
EXPORT int32_t web_chord_note(uint32_t p,uint32_t i){return p<8u&&i<ch_bank.count[p]?ch_bank.notes[p][i]:-1;}
EXPORT uint32_t web_voice_gate(uint32_t group,uint32_t note){uint32_t v,n=0;for(v=0;v<NV;v++)if(voice[v].group==group&&voice[v].note==note&&voice[v].gate)n++;return n;}
EXPORT uint32_t web_voice_tape(uint32_t group,uint32_t note){uint32_t v;for(v=0;v<NV;v++)if(voice[v].group==group&&voice[v].note==note&&voice[v].gate)return voice[v].set;return 255u;}
EXPORT void web_chord_fail(uint32_t n){ch_store_fail_after=n;}
#endif
