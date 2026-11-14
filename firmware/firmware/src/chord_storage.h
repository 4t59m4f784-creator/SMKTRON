/* SPDX-License-Identifier: GPL-3.0-only
 * SC1 scenes: two CRC-checked copies in owned VM, commit header last.
 * No calls from audio ISR; reads at boot, writes only on explicit long pad.
 * The browser uses the same storage code over an emulated NOR image. */
#if defined(DINGHY_WEB)
static uint8_t ch_store_flash[8192];
static uint32_t ch_store_flash_ready, ch_store_fail_after=0xFFFFFFFFu;
static void ch_store_io_init(void){if(!ch_store_flash_ready){memset(ch_store_flash,255,sizeof ch_store_flash);ch_store_flash_ready=1;}}
static int ch_store_read(uint32_t off,void *p,uint32_t n){ch_store_io_init();if(off>8192u||n>8192u-off)return -1;memcpy(p,ch_store_flash+off,n);return 0;}
static int ch_store_write(uint32_t off,const uint8_t *p,uint32_t n){uint32_t i;ch_store_io_init();if(off>8192u||n>8192u-off)return -1;for(i=0;i<n;i++){if(!ch_store_fail_after)return -1;if(ch_store_fail_after!=0xFFFFFFFFu)ch_store_fail_after--;ch_store_flash[off+i]&=p[i];}return 0;}
static int ch_store_erase(uint32_t off){uint32_t i;ch_store_io_init();if(off!=0&&off!=4096u)return -1;for(i=0;i<4096u;i++){if(!ch_store_fail_after)return -1;if(ch_store_fail_after!=0xFFFFFFFFu)ch_store_fail_after--;ch_store_flash[off+i]=255;}return 0;}
#else
static int ch_store_read(uint32_t off,void *p,uint32_t n){return !flash_ok?-1:st_read(CHORDS_AT+off,p,n);}
static int ch_store_write(uint32_t off,const uint8_t *p,uint32_t n){return !flash_ok?-1:fl_write(CHORDS_AT+off,p,n);}
static int ch_store_erase(uint32_t off){uint32_t took;return !flash_ok?-1:fl_erase4k(CHORDS_AT+off,&took);}
#endif
static uint32_t ch_store_disk_bank,ch_store_generation,ch_store_store_writes;
static uint32_t ch_store_crc(uint32_t c,const uint8_t *p,uint32_t n){uint32_t k;while(n--){c^=*p++;for(k=0;k<8u;k++)c=c&1u?(c>>1)^0xEDB88320u:c>>1;}return c;}
static int ch_store_check(uint32_t bank,uint32_t n,uint32_t *generation){
 uint32_t h[5],off=bank*4096u,k,c=0xFFFFFFFFu,left=n;uint8_t b[256];
 if(ch_store_read(off,h,sizeof h)||h[0]!=0x31444843u||h[2]!=n||~ch_store_crc(0xFFFFFFFFu,(uint8_t*)h,16)!=h[4])return -1;
 off+=sizeof h;
 while(left){k=left>sizeof b?sizeof b:left;if(ch_store_read(off,b,k))return -1;c=ch_store_crc(c,b,k);off+=k;left-=k;}
 if(~c!=h[3])return -1;*generation=h[1];return 0;
}
static int ch_store_store_load(void *p,uint32_t n){
 uint32_t a=0,b=0;int va=!ch_store_check(0,n,&a),vb=!ch_store_check(1,n,&b);
 if(!va&&!vb)return -1;
 ch_store_disk_bank=vb&&(!va||(int32_t)(b-a)>0)?1u:0u;ch_store_generation=ch_store_disk_bank?b:a;
 return ch_store_read(ch_store_disk_bank*4096u+20u,p,n);
}
static int ch_store_store_save(const void *p,uint32_t n){
 uint32_t h[5],check,bank=ch_store_disk_bank^1u,off=bank*4096u;
 if(n>4076u)return -1;
 h[0]=0x31444843u;h[1]=ch_store_generation+1u;h[2]=n;h[3]=~ch_store_crc(0xFFFFFFFFu,p,n);h[4]=~ch_store_crc(0xFFFFFFFFu,(uint8_t*)h,16u);
 if(ch_store_erase(off)||ch_store_write(off+20u,p,n)||ch_store_write(off,(uint8_t*)h,sizeof h)||ch_store_check(bank,n,&check))return -1;
 ch_store_disk_bank=bank;ch_store_generation=h[1];ch_store_store_writes++;return 0;
}
