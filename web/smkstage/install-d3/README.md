# SMKSTAGE B3 Alpha 0.1.3-D3 — 10 kHz RAM ISR diagnostic

Only SMK-37 Pro, experimental, hardware untested. The D2 features remain unchanged: Dattorro plate, Leslie controls, tonewheels, modulation wheel, factory presets and library, full favorites and flash journal. D3 adds one isolated test: TIMER5 IRQ entry/exit wrapper executes from internal RAM instead of XIP flash; C key scanner stays at 10 kHz and audio two-core processing and interrupt priorities are unchanged.

Verified JieLi build: https://github.com/4t59m4f784-creator/SMKSYNTH/actions/runs/45334820181
Source: https://github.com/4t59m4f784-creator/SMKSYNTH/tree/smkstage-b3-d3-irq-isolation
Size 651339 B, SHA-256: 02504176970bc7d34b04cb0af2a23c1a56accfa4f8af67a442102e7557157405. Model identity SMK-37 Pro_19703.
No public stable release. Exact package pin required; CRC, model, USB state and consent validated by installer. Never write in USB Rescue (_000); use pinned 0.1.1 fallback in the same browser. If it crashes, photograph VEC/PC/RETS/DBG/EMU/T63/SP; the timer vector should now point to internal RAM (01C0xxxx), which tests the XIP fetch hypothesis.
