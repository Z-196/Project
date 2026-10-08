#ifndef __W25Q64_H
#define __W25Q64_H

#include "stm32f10x.h"                  // Device header
#include "MySPI.h"
#include "W25Q64_Ins.h"
#include <stddef.h>						// 包含NULL的声明

#define W25Q64_CONFIG_ADDR           0x000000  // 配置存储起始地址（4KB扇区）
#define CONFIG_WRITE_MIN_INTERVAL    500       // 配置写入最小间隔(ms)

#include "main.h"

void W25Q64_Init(void);
void W25Q64_ReadID(uint8_t *MID, uint16_t *DID);
void W25Q64_PageProgram(uint32_t Address, uint8_t *DataArray, uint16_t Count);
void W25Q64_SectorErase(uint32_t Address);
void W25Q64_ReadData(uint32_t Address, uint8_t *DataArray, uint32_t Count);

void W25Q64_WriteEnable(void);


#endif
