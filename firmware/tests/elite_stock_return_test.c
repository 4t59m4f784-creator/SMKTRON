/* SPDX-License-Identifier: GPL-3.0-only */
#define main ota_test_main
#include "ota_test.c"
#undef main
#include <assert.h>
int main(int argc,char **argv){
 assert(argc==2);FILE*f=fopen(argv[1],"rb");assert(f);uint8_t*raw=malloc(0x200000);size_t n=fread(raw,1,0x200000,f);fclose(f);
 logical=malloc(n);for(unsigned i=0;i<20;i++)memcpy(logical+i*47,raw+i*48,47);memcpy(logical+940,raw+960,n-960);logical_len=n-20;
 reset_dev();assert(ota_session()==0&&committed&&!bad_erase);assert(!memcmp(committed_parm+8,"ota-SMK-37 Elite_016",20));
 reset_dev();nor[100]^=1;assert(ota_session()==-5&&!committed&&area_erased());
 puts("PASS: Elite OTA accepts official return, commits Elite_016 record, rejects foreign head; physical return still untested");return 0;
}
