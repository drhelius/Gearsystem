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