/* SPDX-License-Identifier: GPL-3.0-only */
/* Main-loop producer, 10 kHz timer consumer. Never waits for a UART byte. */
#define SMK_TX_BYTES 512u
static uint8_t smk_tx_bytes[SMK_TX_BYTES];
static volatile uint32_t smk_tx_w, smk_tx_r;
static unsigned smk_tx_busy;
static volatile uint32_t smk_tx_polls,smk_tx_acked;
#ifndef SMK_TX_TICKS
#define SMK_TX_TICKS() fm1_ticks()
#define SMK_TX_PIN_HIGH() smk_uart1_tx_diag(1)
#endif
static volatile uint32_t smk_tx_min_us,smk_tx_pin_high,smk_tx_pin_low;
static uint32_t smk_tx_started;
static int smk_tx_ready(void) { return smk_tx_w-smk_tx_r <= SMK_TX_BYTES-3u; }
static void smk_tx_message(unsigned st,unsigned a,unsigned b)
{
    unsigned w=smk_tx_w;
    smk_tx_bytes[w++%SMK_TX_BYTES]=(uint8_t)st;
    smk_tx_bytes[w++%SMK_TX_BYTES]=(uint8_t)(a&127u);
    if((st&0xE0u)!=0xC0u)smk_tx_bytes[w++%SMK_TX_BYTES]=(uint8_t)(b&127u);
    RING_PUBLISH();smk_tx_w=w;
}
static void smk_tx_poll(void)
{
    smk_tx_polls++;
    if(smk_tx_busy){
        if(SMK_TX_PIN_HIGH())smk_tx_pin_high++;else smk_tx_pin_low++;
        if(!smk_uart1_tx_done())return;
        uint32_t us=(SMK_TX_TICKS()-smk_tx_started)/24u;
        if(!smk_tx_acked||us<smk_tx_min_us)smk_tx_min_us=us;
        smk_tx_acked++;smk_tx_busy=0;
    }
    if(smk_tx_r==smk_tx_w){smk_tx_busy=0;return;}
    smk_tx_started=SMK_TX_TICKS();
    smk_uart1_tx_put(smk_tx_bytes[smk_tx_r++%SMK_TX_BYTES]);smk_tx_busy=1;
}
