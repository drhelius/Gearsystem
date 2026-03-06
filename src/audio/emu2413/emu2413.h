/*
**
** File: ym2413.c - software implementation of YM2413
**                  FM sound generator type OPLL
**
** Copyright (C) 2002 Jarek Burczynski
**
** Version 1.0
**
*/

#ifndef _H_YM2413_
#define _H_YM2413_

#include <stdint.h>

typedef struct
{
  uint32_t  ar;
  uint32_t  dr;
  uint32_t  rr;
  uint8_t   KSR;
  uint8_t   ksl;
  uint8_t   ksr;
  uint8_t   mul;
  uint32_t  phase;
  uint32_t  freq;
  uint8_t   fb_shift;
  int32_t   op1_out[2];
  uint8_t   eg_type;
  uint8_t   state;
  uint32_t  TL;
  int32_t   TLL;
  int32_t   volume;
  uint32_t  sl;
  uint8_t   eg_sh_dp;
  uint8_t   eg_sel_dp;
  uint8_t   eg_sh_ar;
  uint16_t  eg_sel_ar;
  uint8_t   eg_sh_dr;
  uint8_t   eg_sel_dr;
  uint8_t   eg_sh_rr;
  uint8_t   eg_sel_rr;
  uint8_t   eg_sh_rs;
  uint8_t   eg_sel_rs;
  uint32_t  key;
  uint32_t  AMmask;
  uint8_t   vib;
  unsigned int wavetable;
} YM2413_OPLL_SLOT;

typedef struct
{
  YM2413_OPLL_SLOT SLOT[2];
  uint32_t  block_fnum;
  uint32_t  fc;
  uint32_t  ksl_base;
  uint8_t   kcode;
  uint8_t   sus;
} YM2413_OPLL_CH;

typedef struct {
  YM2413_OPLL_CH P_CH[9];
  uint8_t   instvol_r[9];
  uint32_t  eg_cnt;
  uint32_t  eg_timer;
  uint32_t  eg_timer_add;
  uint32_t  eg_timer_overflow;
  uint8_t   rhythm;
  uint32_t  lfo_am_cnt;
  uint32_t  lfo_am_inc;
  uint32_t  lfo_pm_cnt;
  uint32_t  lfo_pm_inc;
  uint32_t  noise_rng;
  uint32_t  noise_p;
  uint32_t  noise_f;
  uint8_t   inst_tab[19][8];
  uint32_t  fn_tab[1024];
  uint8_t   address;
  uint8_t   status;
  double    clock;
  int       rate;
} YM2413_OPLL;

#ifdef __cplusplus
extern "C" {
#endif

extern void YM2413Init(void);
extern void YM2413ResetChip(void);
extern int YM2413Update(void);
extern void YM2413Write(unsigned int a, unsigned int v);
extern unsigned int YM2413Read(void);
extern unsigned char *YM2413GetContextPtr(void);
extern unsigned int YM2413GetContextSize(void);

#ifdef __cplusplus
}
#endif

#endif /*_H_YM2413_*/