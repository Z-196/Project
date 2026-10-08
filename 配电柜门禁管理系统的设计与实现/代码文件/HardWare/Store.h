#ifndef __STORE_H
#define __STORE_H
#include "stm32f10x.h"                  // Device header
#include "MyFLASH.h"
#include <string.h>

extern uint16_t Store_Data[];

void Store_Init(void);
void Store_Save(void);
void Store_Clear(void);

void Store_SaveParamsToCache(void);
void Store_LoadParamsFromCache(void);

#endif
