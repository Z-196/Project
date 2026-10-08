#include "Int_MPU6050.h"

// 保存偏移量的值
int32_t acc_x_offset = 0;
int32_t acc_y_offset = 0;
int32_t acc_z_offset = 0;
int32_t gyro_x_offset = 0;
int32_t gyro_y_offset = 0;
int32_t gyro_z_offset = 0;
/**
 * @brief 写MPU6050寄存器
 *
 * @param reg 寄存器地址
 * @param data 写入寄存器的值
 */
static void Int_MPU6050_Write_Reg(uint8_t reg, uint8_t data)
{
    // 1.句柄 2.从机地址 3.寄存器地址 4.寄存器地址长度 5.写入的数据地址 6.写入的字节个数 7.超时时间
    HAL_I2C_Mem_Write(&hi2c2, MPU6050_ADDR, reg, I2C_MEMADD_SIZE_8BIT, &data, 1, 1000);
}

/**
 * @brief 读MPU6050寄存器（单字节）
 *
 * @param reg 要读取的寄存器的地址
 * @param data 读出寄存器的值的存放地址
 */
static void Int_MPU6050_Read_Reg(uint8_t reg, uint8_t *data)
{
    // 1.句柄 2.从机地址 3.寄存器地址 4.寄存器地址长度 5.写入的数据地址 6.写入的字节个数 7.超时时间
    HAL_I2C_Mem_Read(&hi2c2, MPU6050_ADDR, reg, I2C_MEMADD_SIZE_8BIT, data, 1, 1000);
}

/**
 * @brief 读MPU6050寄存器（多字节）
 *
 * @param reg 要读取的寄存器的地址
 * @param data 读出寄存器的值的存放地址
 */
static void Int_MPU6050_Read_Regs(uint8_t reg, uint8_t *data, uint8_t len)
{
    HAL_I2C_Mem_Read(&hi2c2, MPU6050_ADDR, reg, I2C_MEMADD_SIZE_8BIT, data, len, 1000);
}

/**
 * @brief 在初始化MPU6050完成之后，对MPU6050进行零偏校准
 */
void Int_MPU6050_calculate_offset(void)
{
    // 1. 等待飞机停放平稳
    // 判断飞机是否停放平稳的标准：前后两次加速度的值差值小于200 连续100次
    Accel_Struct current_accel = {0};
    Accel_Struct last_accel = {0};
    Int_MPU6050_Get_Acc(&last_accel);
    uint8_t count = 0;
    while (count < 100)
    {
        Int_MPU6050_Get_Acc(&current_accel);
        // 判断飞机是否平稳 => 若选用的参数过小 => 会造成一直无法判断为平稳
        if (abs(current_accel.accel_x - last_accel.accel_x) < 1000 &&
            abs(current_accel.accel_y - last_accel.accel_y) < 1000 &&
            abs(current_accel.accel_z - last_accel.accel_z) < 1000)
        {
            count++;
        }
        else
        {
            count = 0;
        }
        last_accel = current_accel;
        vTaskDelay(6);
    }

    // 2. 飞机已经平稳，开始进行零偏校准
    Gyro_Accel_Struct gyro_accel_data = {0};
    int32_t acc_x_sum = 0;
    int32_t acc_y_sum = 0;
    int32_t acc_z_sum = 0;
    int32_t gyro_x_sum = 0;
    int32_t gyro_y_sum = 0;
    int32_t gyro_z_sum = 0;

    for (int i = 0; i < 100; i++)
    {
        // 重新读取加速度和角速度
        Int_MPU6050_Get_Data(&gyro_accel_data);
        acc_x_sum += (gyro_accel_data.accel.accel_x - 0);
        acc_y_sum += (gyro_accel_data.accel.accel_y - 0);
        acc_z_sum += (gyro_accel_data.accel.accel_z - 16384);
        gyro_x_sum += (gyro_accel_data.gyro.gyro_x - 0);
        gyro_y_sum += (gyro_accel_data.gyro.gyro_y - 0);
        gyro_z_sum += (gyro_accel_data.gyro.gyro_z - 0);

        vTaskDelay(6);
    }
    acc_x_offset = acc_x_sum / 100;
    acc_y_offset = acc_y_sum / 100;
    acc_z_offset = acc_z_sum / 100;
    gyro_x_offset = gyro_x_sum / 100;
    gyro_y_offset = gyro_y_sum / 100;
    gyro_z_offset = gyro_z_sum / 100;
}

/**
 * @brief 初始化MPU6050芯片
 */
void Int_MPU6050_Init(void)
{
    // 1. 重启芯片，重置所有寄存器的值 => 写电源管理寄存器1 => DEVICE_RESET
    Int_MPU6050_Write_Reg(MPU6050_PWR_MGMT_1, 0x80);
    vTaskDelay(100);
    // 重置完成之后，0x6B寄存器的值是0x40，表示当前为低功耗模式
    // uint8_t data = 0;
    // while (data != 0x40)
    // {
    //     Int_MPU6050_Read_Reg(MPU6050_PWR_MGMT_1, &data);
    // }

    // 唤醒MPU6050 => 进入到正常工作状态 && 配置使用的系统时钟为PLL的X轴GYRO
    Int_MPU6050_Write_Reg(MPU6050_PWR_MGMT_1, 0x01);
    vTaskDelay(100);

    // 2. 选择合适的量程 => 在够用的范围内，选择的越小越好 => 精度高
    // 2.1 填写角速度量程为±2000°/s
    Int_MPU6050_Write_Reg(MPU6050_GYRO_CONFIG, 3 << 3);

    // 2.2 填写加速度量程为±2g
    Int_MPU6050_Write_Reg(MPU6050_ACCEL_CONFIG, 0x00);

    // 3. 关闭中断使能
    Int_MPU6050_Write_Reg(MPU6050_INT_ENABLE, 0x00);

    // 4. 用户配置寄存器 => 不使用FIFO队列、不适用扩展的I2C
    Int_MPU6050_Write_Reg(MPU6050_USER_CTRL, 0x00);

    // 5. 设置采样频率 => 陀螺仪监控三轴加速度、三轴角速度 => 默认频率1000HZ => 1ms读取一次
    // 基本逻辑 => 采样率必须大于后续数据的使用频率，否则失真 => 香农定理：采样率 >= 2倍使用频率
    // 6ms读取一次数据，使用频率约为166HZ => 设置采样分频为2，填写的值为2-1 => 采样频率为500HZ
    Int_MPU6050_Write_Reg(MPU6050_SMPLRT_DIV, 0x01);

    // 6. 设置低通滤波的值为184HZ 188HZ => 1
    Int_MPU6050_Write_Reg(MPU6050_CONFIG, 0x01);

    // 7. 使能加速度传感器和角速度传感器
    Int_MPU6050_Write_Reg(MPU6050_PWR_MGMT_2, 0x00);

}

/**
 * @brief 读取三轴角速度 => 需要进行零偏校准 => 本身抖动不严重
 *
 * @param gyro 三轴角速度结构体指针
 */
void Int_MPU6050_Get_Gyro(Gyro_Struct *gyro)
{
    // 存储角速度的寄存器地址从0x43开始，高8位在前
    uint8_t data[6] = {0};
    Int_MPU6050_Read_Regs(0x43, data, 6);
    gyro->gyro_x = (int16_t)((data[0] << 8) | data[1]) - gyro_x_offset;
    gyro->gyro_y = (int16_t)((data[2] << 8) | data[3]) - gyro_y_offset;
    gyro->gyro_z = (int16_t)((data[4] << 8) | data[5]) - gyro_z_offset;
}

/**
 * @brief 读取三轴加速度 => 需要进行零偏校准 => 抖动比较严重
 *
 * @param acc 三轴加速度结构体指针
 */
void Int_MPU6050_Get_Acc(Accel_Struct *acc)
{
    // 存储加速度的寄存器地址从0x3B开始，高8位在前
    uint8_t data[6] = {0};
    Int_MPU6050_Read_Regs(0x3B, data, 6);
    acc->accel_x = (int16_t)((data[0] << 8) | data[1]) - acc_x_offset;
    acc->accel_y = (int16_t)((data[2] << 8) | data[3]) - acc_y_offset;
    acc->accel_z = (int16_t)((data[4] << 8) | data[5]) - acc_z_offset;
}

/**
 * @brief 获取所有的六轴数据
 *
 * @param data 六轴数据结构体指针
 */
void Int_MPU6050_Get_Data(Gyro_Accel_Struct *data)
{
    Int_MPU6050_Get_Gyro(&data->gyro);
    Int_MPU6050_Get_Acc(&data->accel);
}
