/* SPDX-License-Identifier: GPL-3.0-only. Two independent CRC sectors, header committed last. */
#if defined(DINGHY_WEB)
static uint8_t mm_flash[8192];static uint32_t mm_flash_ready,mm_fail_after=0xFFFFFFFFu,mm_writes;
static void mm_io_init(void){if(!mm_flash_ready){memset(mm_flash,255,sizeof mm_flash);mm_flash_ready=1;}}
static int mm_read(unsigned off,void *p,unsigned n){mm_io_init();if(off>8192u||n>8192u-off)return -1;memcpy(p,mm_flash+off,n);return 0;}
static int mm_write(unsigned off,const uint8_t *p,unsigned n){mm_io_init();if(off>8192u||n>8192u-off)return -1;while(n--){if(!mm_fail_after)return -1;if(mm_fail_after!=0xFFFFFFFFu)mm_fail_after--;mm_flash[off++]&=*p++;}return 0;}
static int mm_erase(unsigned off){mm_io_init();if(off!=0&&off!=4096u)return -1;for(unsigned i=0;i<4096;i++){if(!mm_fail_after)return -1;if(mm_fail_after!=0xFFFFFFFFu)mm_fail_after--;mm_flash[off+i]=255;}return 0;}
#else
static int mm_read(unsigned off,void *p,unsigned n){return !flash_ok?-1:st_read(MIDI_MAP_AT+off,p,n);}
static int mm_write(unsigned off,const uint8_t *p,unsigned n){return !flash_ok?-1:fl_write(MIDI_MAP_AT+off,p,n);}
static int mm_erase(unsigned off){uint32_t took;return !flash_ok?-1:fl_erase4k(MIDI_MAP_AT+off,&took);}
#endif
static uint32_t mm_crc(const uint8_t *p,unsigned n){uint32_t c=~0u;while(n--){c^=*p++;for(unsigned k=0;k<8;k++)c=c&1u?(c>>1)^0xEDB88320u:c>>1;}return ~c;}
static int mm_store_load(unsigned i,void *p,unsigned n){
 uint32_t h[3];if(i>1||n>4084)return -1;unsigned off=i*4096u;
 if(mm_read(off,h,sizeof h)||h[0]!=0x314D4D54u||h[1]!=n||mm_read(off+12,p,n)||mm_crc(p,n)!=h[2])return -1;return 0;
}
static int mm_store_save(unsigned i,const void *p,unsigned n){
 uint32_t h[3]={0x314D4D54u,n,0};if(i>1||n>4084)return -1;h[2]=mm_crc(p,n);unsigned off=i*4096u;
 if(mm_erase(off)||mm_write(off+12,p,n)||mm_write(off,(const uint8_t*)h,sizeof h))return -1;
#if defined(DINGHY_WEB)
 mm_writes++;
#endif
 return 0;
}
