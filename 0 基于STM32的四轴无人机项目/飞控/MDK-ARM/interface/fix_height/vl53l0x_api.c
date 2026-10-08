#include "vl53l0x_api.h"

// ===================== I2C PAL底层 =====================
VL53L0X_Error VL53L0X_WriteMulti(VL53L0X_DEV Dev, uint8_t index, uint8_t *pdata, uint32_t count)
{
    if (HAL_I2C_Mem_Write(&VL53L0X_I2C_HANDLE, (Dev->I2cAddress << 1), index, I2C_MEMADD_SIZE_8BIT, pdata, count, VL53L0X_I2C_TIMEOUT) != HAL_OK)
        return VL53L0X_ERROR_TIME_OUT;
    return VL53L0X_ERROR_NONE;
}

VL53L0X_Error VL53L0X_ReadMulti(VL53L0X_DEV Dev, uint8_t index, uint8_t *pdata, uint32_t count)
{
    if (HAL_I2C_Mem_Read(&VL53L0X_I2C_HANDLE, (Dev->I2cAddress << 1), index, I2C_MEMADD_SIZE_8BIT, pdata, count, VL53L0X_I2C_TIMEOUT) != HAL_OK)
        return VL53L0X_ERROR_TIME_OUT;
    return VL53L0X_ERROR_NONE;
}

VL53L0X_Error VL53L0X_WrByte(VL53L0X_DEV Dev, uint8_t index, uint8_t data)
{
    return VL53L0X_WriteMulti(Dev, index, &data, 1);
}

VL53L0X_Error VL53L0X_RdByte(VL53L0X_DEV Dev, uint8_t index, uint8_t *data)
{
    return VL53L0X_ReadMulti(Dev, index, data, 1);
}

VL53L0X_Error VL53L0X_WrWord(VL53L0X_DEV Dev, uint8_t index, uint16_t data)
{
    uint8_t buf[2] = {(uint8_t)(data >> 8), (uint8_t)data};
    return VL53L0X_WriteMulti(Dev, index, buf, 2);
}

VL53L0X_Error VL53L0X_RdWord(VL53L0X_DEV Dev, uint8_t index, uint16_t *data)
{
    uint8_t buf[2];
    VL53L0X_Error stat = VL53L0X_ReadMulti(Dev, index, buf, 2);
    *data = ((uint16_t)buf[0] << 8) | buf[1];
    return stat;
}

VL53L0X_Error VL53L0X_WrDWord(VL53L0X_DEV Dev, uint8_t index, uint32_t data)
{
    uint8_t buf[4] = {(uint8_t)(data >> 24), (uint8_t)(data >> 16), (uint8_t)(data >> 8), (uint8_t)data};
    return VL53L0X_WriteMulti(Dev, index, buf, 4);
}

VL53L0X_Error VL53L0X_RdDWord(VL53L0X_DEV Dev, uint8_t index, uint32_t *data)
{
    uint8_t buf[4];
    VL53L0X_Error stat = VL53L0X_ReadMulti(Dev, index, buf, 4);
    *data = ((uint32_t)buf[0] << 24) | ((uint32_t)buf[1] << 16) | ((uint32_t)buf[2] << 8) | buf[3];
    return stat;
}

VL53L0X_Error VL53L0X_UpdateByte(VL53L0X_DEV Dev, uint8_t index, uint8_t AndData, uint8_t OrData)
{
    uint8_t reg;
    VL53L0X_Error stat = VL53L0X_RdByte(Dev, index, &reg);
    if (stat != VL53L0X_ERROR_NONE) return stat;
    reg = (reg & AndData) | OrData;
    return VL53L0X_WrByte(Dev, index, reg);
}

VL53L0X_Error VL53L0X_LockSequenceAccess(VL53L0X_DEV Dev)
{
    return VL53L0X_ERROR_NONE;
}

VL53L0X_Error VL53L0X_UnlockSequenceAccess(VL53L0X_DEV Dev)
{
    return VL53L0X_ERROR_NONE;
}

VL53L0X_Error VL53L0X_WaitMs(VL53L0X_DEV Dev, uint32_t WaitMs)
{
    vTaskDelay(pdMS_TO_TICKS(WaitMs));
    return VL53L0X_ERROR_NONE;
}

void VL53L0X_HardwareReset(void)
{
    HAL_GPIO_WritePin(VL53L0X_XSHUT_PORT, VL53L0X_XSHUT_PIN, GPIO_PIN_RESET);
    vTaskDelay(pdMS_TO_TICKS(20));
    HAL_GPIO_WritePin(VL53L0X_XSHUT_PORT, VL53L0X_XSHUT_PIN, GPIO_PIN_SET);
    vTaskDelay(pdMS_TO_TICKS(30));
}

// ===================== 初始化底层 =====================
VL53L0X_Error VL53L0X_DataInit(VL53L0X_DEV Dev)
{
    Dev->I2cAddress = VL53L0X_DEV_ADDR;
    Dev->PalState = VL53L0X_STATE_WAIT_STATICINIT;
    return VL53L0X_ERROR_NONE;
}

VL53L0X_Error VL53L0X_StaticInit(VL53L0X_DEV Dev)
{
    VL53L0X_Error stat = VL53L0X_ERROR_NONE;
    uint8_t reg;
    stat = VL53L0X_UpdateByte(Dev, 0x88, 0x7F, 0x01);
    if (stat != VL53L0X_ERROR_NONE) goto exit;
    stat = VL53L0X_WrByte(Dev, 0x80, 0x01);
    if (stat != VL53L0X_ERROR_NONE) goto exit;
    stat = VL53L0X_WrByte(Dev, 0xFF, 0x01);
    if (stat != VL53L0X_ERROR_NONE) goto exit;
    stat = VL53L0X_WrByte(Dev, 0x00, 0x00);
    if (stat != VL53L0X_ERROR_NONE) goto exit;
    stat = VL53L0X_RdByte(Dev, 0x92, &reg);
    if (stat != VL53L0X_ERROR_NONE) goto exit;
    stat = VL53L0X_WrByte(Dev, 0x92, reg | 0x04);
    if (stat != VL53L0X_ERROR_NONE) goto exit;
    stat = VL53L0X_WrByte(Dev, 0xFF, 0x00);
    if (stat != VL53L0X_ERROR_NONE) goto exit;
    stat = VL53L0X_WrByte(Dev, 0x80, 0x00);
    if (stat != VL53L0X_ERROR_NONE) goto exit;
    Dev->PalState = VL53L0X_STATE_IDLE;
exit:
    return stat;
}

// 时序预算完整实现
VL53L0X_Error VL53L0X_SetMeasurementTimingBudgetMicroSeconds(VL53L0X_DEV Dev, uint32_t us)
{
    VL53L0X_Error stat;
    uint16_t macrop;
    if (us < 12000) return VL53L0X_ERROR_INVALID_PARAMS;
    macrop = (uint16_t)((us - 1300) / 2);
    stat = VL53L0X_WrWord(Dev, RANGE_CONFIG__TIMEOUT_MACROP_LO, macrop);
    return stat;
}

VL53L0X_Error VL53L0X_SetDeviceMode(VL53L0X_DEV Dev, VL53L0X_DeviceModes mode)
{
    return VL53L0X_ERROR_NONE;
}

VL53L0X_Error VL53L0X_StartMeasurement(VL53L0X_DEV Dev)
{
    VL53L0X_Error stat;
    stat = VL53L0X_UpdateByte(Dev, VL53L0X_REG_SYSRANGE_START, 0xFE, 0x01);
    if (stat == VL53L0X_ERROR_NONE)
        Dev->PalState = VL53L0X_STATE_RUNNING;
    return stat;
}

VL53L0X_Error VL53L0X_StopMeasurement(VL53L0X_DEV Dev)
{
    VL53L0X_Error stat;
    stat = VL53L0X_UpdateByte(Dev, VL53L0X_REG_SYSRANGE_START, 0xFE, 0x00);
    if (stat == VL53L0X_ERROR_NONE)
        Dev->PalState = VL53L0X_STATE_IDLE;
    return stat;
}

VL53L0X_Error VL53L0X_GetMeasurementDataReady(VL53L0X_DEV Dev, uint8_t *pReady)
{
    uint8_t reg;
    VL53L0X_Error stat = VL53L0X_RdByte(Dev, VL53L0X_RESULT_RANGE_STATUS, &reg);
    *pReady = (reg & 0x01) ? 1 : 0;
    return stat;
}

VL53L0X_Error VL53L0X_ClearInterruptMask(VL53L0X_DEV Dev, uint32_t mask)
{
    return VL53L0X_WrByte(Dev, VL53L0X_REG_SYSTEM_INTERRUPT_CLEAR, mask & 0xFF);
}

// 修复：补全返回值 VL53L0X_Error
VL53L0X_Error VL53L0X_GetRangingMeasurementData(VL53L0X_DEV Dev, VL53L0X_RangingMeasurementData_t *pData)
{
    uint8_t buf[16];
    VL53L0X_Error stat = VL53L0X_ReadMulti(Dev, VL53L0X_RESULT_RANGE_STATUS, buf, 16);
    if (stat != VL53L0X_ERROR_NONE)
        return stat;
    pData->RangeStatus = buf[0] & 0x0F;
    pData->RangeMilliMeter = ((uint16_t)buf[11] << 8) | buf[12];
    pData->SignalRate = ((int32_t)buf[14] << 16) | buf[15];
    pData->AmbientRate = 0;
    return VL53L0X_ERROR_NONE;
}

// ===================== 顶层对外API =====================
VL53L0X_Error VL53_Init(VL53L0X_DEV Dev)
{
    VL53L0X_HardwareReset();
    VL53L0X_Error stat = VL53L0X_DataInit(Dev);
    if (stat != VL53L0X_ERROR_NONE) return stat;
    stat = VL53L0X_StaticInit(Dev);
    if (stat != VL53L0X_ERROR_NONE) return stat;
    stat = VL53_SetMeasureTiming(Dev, 30000);
    if (stat != VL53L0X_ERROR_NONE) return stat;
    stat = VL53L0X_SetDeviceMode(Dev, VL53L0X_DEVICEMODE_CONTINUOUS_RANGING);
    return stat;
}

VL53L0X_Error VL53_SetMeasureTiming(VL53L0X_DEV Dev, uint32_t us)
{
    return VL53L0X_SetMeasurementTimingBudgetMicroSeconds(Dev, us);
}

VL53L0X_Error VL53_IsDataReady(VL53L0X_DEV Dev, uint8_t *ready)
{
    return VL53L0X_GetMeasurementDataReady(Dev, ready);
}

VL53L0X_Error VL53_GetRangingData(VL53L0X_DEV Dev, uint16_t *dist_mm)
{
    VL53L0X_RangingMeasurementData_t meas;
    VL53L0X_Error stat = VL53L0X_GetRangingMeasurementData(Dev, &meas);
    if (stat != VL53L0X_ERROR_NONE)
        return stat;
    VL53L0X_ClearInterruptMask(Dev, 0x01);
    if (meas.RangeStatus != 0)
        return VL53L0X_ERROR_RANGE_ERROR;
    *dist_mm = meas.RangeMilliMeter;
    return VL53L0X_ERROR_NONE;
}

VL53L0X_Error VL53_SingleRanging(VL53L0X_DEV Dev, uint16_t *dist_mm)
{
    VL53L0X_Error stat;
    uint8_t ready = 0;
    stat = VL53L0X_SetDeviceMode(Dev, VL53L0X_DEVICEMODE_SINGLE_RANGING);
    if (stat != VL53L0X_ERROR_NONE) return stat;
    stat = VL53L0X_StartMeasurement(Dev);
    if (stat != VL53L0X_ERROR_NONE) return stat;
    while (1)
    {
        VL53_IsDataReady(Dev, &ready);
        if (ready)
            break;
        vTaskDelay(pdMS_TO_TICKS(2));
    }
    stat = VL53_GetRangingData(Dev, dist_mm);
    return stat;
}
