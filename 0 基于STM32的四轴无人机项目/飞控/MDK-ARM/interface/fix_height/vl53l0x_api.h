#ifndef __VL53L0X_API_H__
#define __VL53L0X_API_H__

#include <stdint.h>
#include "i2c.h"
#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"

// ===================== 基础固定类型 =====================
typedef int32_t FixPoint1616_t;

// 错误码枚举
typedef enum {
    VL53L0X_ERROR_NONE = 0,
    VL53L0X_ERROR_INVALID_PARAMS,
    VL53L0X_ERROR_MODE_NOT_SUPPORTED,
    VL53L0X_ERROR_TIME_OUT,
    VL53L0X_ERROR_NOT_IMPLEMENTED,
    VL53L0X_ERROR_RANGE_ERROR,
    VL53L0X_ERROR_REF_SPAD_INIT
} VL53L0X_Error;

// 芯片内部状态
typedef enum {
    VL53L0X_STATE_POWERDOWN,
    VL53L0X_STATE_WAIT_STATICINIT,
    VL53L0X_STATE_STANDBY,
    VL53L0X_STATE_IDLE,
    VL53L0X_STATE_RUNNING,
    VL53L0X_STATE_ERROR
} VL53L0X_State;

// 测距工作模式
typedef enum {
    VL53L0X_DEVICEMODE_SINGLE_RANGING,
    VL53L0X_DEVICEMODE_CONTINUOUS_RANGING,
    VL53L0X_DEVICEMODE_CONTINUOUS_TIMED_RANGING
} VL53L0X_DeviceModes;

// VCSEL脉冲周期
typedef enum {
    VcselPeriodPreRange,
    VcselPeriodFinalRange
} VL53L0X_VcselPeriod;

// 测距序列步骤ID
typedef enum {
    SEQUENCESTEP_TCC,
    SEQUENCESTEP_DSS,
    SEQUENCESTEP_MSRC,
    SEQUENCESTEP_PRE_RANGE,
    SEQUENCESTEP_FINAL_RANGE
} VL53L0X_SequenceStepId;

// 测距结果核心结构体
typedef struct {
    uint8_t RangeStatus;
    uint16_t RangeMilliMeter;
    FixPoint1616_t SignalRate;
    FixPoint1616_t AmbientRate;
} VL53L0X_RangingMeasurementData_t;

// 设备参数结构体
typedef struct {
    uint16_t XTalkCompRate;
    int32_t OffsetMicroMeter;
    uint16_t TimingBudgetUs;
    uint32_t InterMeasMs;
} VL53L0X_DeviceParameters_t;

// 设备句柄
typedef struct VL53L0X_DevData {
    uint8_t I2cAddress;
    VL53L0X_State PalState;
} VL53L0X_DevData;
typedef VL53L0X_DevData *VL53L0X_DEV;

// ===================== 硬件配置 =====================
#define VL53L0X_I2C_HANDLE      hi2c1
#define VL53L0X_DEV_ADDR        0x29U
#define VL53L0X_I2C_TIMEOUT     300U
#define VL53L0X_XSHUT_PORT      VX_XSHUT_GPIO_Port
#define VL53L0X_XSHUT_PIN       VX_XSHUT_Pin

// ===================== 寄存器 =====================
#define VL53L0X_REG_IDENTIFICATION_MODEL_ID         0xC0
#define VL53L0X_REG_SYSRANGE_START                  0x00
#define VL53L0X_REG_SYSTEM_SEQUENCE_CONFIG          0x01
#define VL53L0X_REG_SYSTEM_RANGE_CONFIG             0x09
#define VL53L0X_REG_SYSTEM_INTERMEAS_PERIOD         0x04
#define VL53L0X_REG_SYSTEM_INTERRUPT_CLEAR          0x0B
#define VL53L0X_RESULT_RANGE_STATUS                 0x14
#define VL53L0X_ALGO_PART_TO_PART_RANGE_OFFSET_MM   0x28
#define VL53L0X_GLOBAL_VHV_CONFIG                   0x0E
#define RANGE_CONFIG__TIMEOUT_MACROP_LO            0x29
#define RANGE_CONFIG__TIMEOUT_MACROP_HI            0x2A

// ===================== 底层I2C函数【全部补全返回值】 =====================
VL53L0X_Error VL53L0X_WriteMulti(VL53L0X_DEV Dev, uint8_t index, uint8_t *pdata, uint32_t count);
VL53L0X_Error VL53L0X_ReadMulti(VL53L0X_DEV Dev, uint8_t index, uint8_t *pdata, uint32_t count);
VL53L0X_Error VL53L0X_WrByte(VL53L0X_DEV Dev, uint8_t index, uint8_t data);
VL53L0X_Error VL53L0X_RdByte(VL53L0X_DEV Dev, uint8_t index, uint8_t *data);
VL53L0X_Error VL53L0X_WrWord(VL53L0X_DEV Dev, uint8_t index, uint16_t data);
VL53L0X_Error VL53L0X_RdWord(VL53L0X_DEV Dev, uint8_t index, uint16_t *data);
VL53L0X_Error VL53L0X_WrDWord(VL53L0X_DEV Dev, uint8_t index, uint32_t data);
VL53L0X_Error VL53L0X_RdDWord(VL53L0X_DEV Dev, uint8_t index, uint32_t *data); //修复返回值
VL53L0X_Error VL53L0X_UpdateByte(VL53L0X_DEV Dev, uint8_t index, uint8_t AndData, uint8_t OrData); //修复参数数量&返回值

VL53L0X_Error VL53L0X_LockSequenceAccess(VL53L0X_DEV Dev);
VL53L0X_Error VL53L0X_UnlockSequenceAccess(VL53L0X_DEV Dev);
VL53L0X_Error VL53L0X_WaitMs(VL53L0X_DEV Dev, uint32_t WaitMs);
void VL53L0X_HardwareReset(void);

// ===================== 内部底层API【全部补全返回值】 =====================
VL53L0X_Error VL53L0X_DataInit(VL53L0X_DEV Dev);
VL53L0X_Error VL53L0X_StaticInit(VL53L0X_DEV Dev); //修复返回值缺失
VL53L0X_Error VL53L0X_SetMeasurementTimingBudgetMicroSeconds(VL53L0X_DEV Dev, uint32_t us);
VL53L0X_Error VL53L0X_SetDeviceMode(VL53L0X_DEV Dev, VL53L0X_DeviceModes mode);
VL53L0X_Error VL53L0X_StartMeasurement(VL53L0X_DEV Dev);
VL53L0X_Error VL53L0X_StopMeasurement(VL53L0X_DEV Dev);
VL53L0X_Error VL53L0X_GetMeasurementDataReady(VL53L0X_DEV Dev, uint8_t *pReady);
VL53L0X_Error VL53L0X_ClearInterruptMask(VL53L0X_DEV Dev, uint32_t mask);
VL53L0X_Error VL53L0X_GetRangingMeasurementData(VL53L0X_DEV Dev, VL53L0X_RangingMeasurementData_t *pData); //修复返回值

// ===================== 对外顶层API =====================
VL53L0X_Error VL53_Init(VL53L0X_DEV Dev);
VL53L0X_Error VL53_SetMeasureTiming(VL53L0X_DEV Dev, uint32_t us);
VL53L0X_Error VL53_IsDataReady(VL53L0X_DEV Dev, uint8_t *ready);
VL53L0X_Error VL53_GetRangingData(VL53L0X_DEV Dev, uint16_t *dist_mm);
VL53L0X_Error VL53_SingleRanging(VL53L0X_DEV Dev, uint16_t *dist_mm);

#endif // __VL53L0X_API_H__
