#include "APP_flight.h"

Gyro_Accel_Struct gyro_accel_data = {0};
Gyro_Struct last_gyro = {0};
Euler_Struct euler_angle = {0};
float gyro_z_sum = 0;

extern Remote_Data_Struct remote_data; // 存储当前接收的遥控数据
extern Flight_State_Enum flight_state; // 表示当前的飞行状态
extern TaskHandle_t com_task_handle;
extern uint16_t fix_height;     // 进入定高模式时，飞机所处的高度
extern uint16_t height_control; // 进入高度调制模式时，飞机当前所处的高度
extern uint16_t now_height;     // 飞机目前所处的高度
float last_height = 0;          // 上一次的高度值

// 电机结构体
Motor_Struct left_top_motor = {.tim = &htim1, .channel = MOTOR_L1, .speed = 0};
Motor_Struct left_bottom_motor = {.tim = &htim1, .channel = MOTOR_L2, .speed = 0};
Motor_Struct right_top_motor = {.tim = &htim1, .channel = MOTOR_R1, .speed = 0};
Motor_Struct right_bottom_motor = {.tim = &htim1, .channel = MOTOR_R2, .speed = 0};

// PID调参 => 先调节内环，再调节外环 => 外环对应欧拉角，内环对应角速度
// 俯仰角PID结构体
PID_Struct pitch_outer_pid = {.kp = -6.5, .ki = 0.0, .kd = 0.00, .dt = PID_PERIOD};
PID_Struct pitch_inner_pid = {.kp = -3.0, .ki = 0.0, .kd = -0.50, .dt = PID_PERIOD};
// 横滚角PID结构体
PID_Struct roll_outer_pid = {.kp = -6.5, .ki = 0.0, .kd = 0.00, .dt = PID_PERIOD};
PID_Struct roll_inner_pid = {.kp = -3.0, .ki = 0.0, .kd = -0.50, .dt = PID_PERIOD};
// 偏航角PID结构体
PID_Struct yaw_outer_pid = {.kp = -2.7, .ki = 0.0, .kd = 0.00, .dt = PID_PERIOD};
PID_Struct yaw_inner_pid = {.kp = 4.0, .ki = 0.0, .kd = 0.00, .dt = PID_PERIOD};
// 定高PID结构体
PID_Struct height_pid = {.kp = -0.80, .ki = -0.0, .kd = -0.15, .integral_limit = 80, .dt = 0.030}; // 时间间隔为30ms
int16_t HEIGHT_CONTROL_THRESHOLD = 200;                                                            // 高度调制模式下，悬停所需的基础油门值

/**
 * @brief 飞控任务初始化
 */
void App_flight_init(void)
{
    // MPU6050初始化
    Int_MPU6050_Init();
    for (int i = 0; i < 5; i++)
    {
        Int_MPU6050_calculate_offset();
    }

    // 初始化激光测距仪
    // Int_VL53L1X_Init();
    Int_VL53L0X_Init();

    // 电机初始化
    Int_motor_start(&left_top_motor);
    Int_motor_start(&left_bottom_motor);
    Int_motor_start(&right_top_motor);
    Int_motor_start(&right_bottom_motor);
}
/**
 * @brief 根据陀螺仪测量的数据，计算出欧拉角
 */
void App_flight_get_euler(void)
{
    // 1. 使用MPU6050的硬件接口 => 得到六轴数据
    Int_MPU6050_Get_Data(&gyro_accel_data);

    // 2. 对角速度进行低通滤波 =>  准确性不高，但计算迅速
    gyro_accel_data.gyro.gyro_x = Com_Filter_LowPass(gyro_accel_data.gyro.gyro_x, last_gyro.gyro_x);
    gyro_accel_data.gyro.gyro_y = Com_Filter_LowPass(gyro_accel_data.gyro.gyro_y, last_gyro.gyro_y);
    gyro_accel_data.gyro.gyro_z = Com_Filter_LowPass(gyro_accel_data.gyro.gyro_z, last_gyro.gyro_z);
    last_gyro.gyro_x = gyro_accel_data.gyro.gyro_x;
    last_gyro.gyro_y = gyro_accel_data.gyro.gyro_y;
    last_gyro.gyro_z = gyro_accel_data.gyro.gyro_z;
    // 打印角速度
    // debug_printf(":%d,%d,%d\n", gyro_accel_data.gyro_x, gyro_accel_data.gyro_y, gyro_accel_data.gyro_z);

    // 3. 对波动变化大的加速度进行卡尔曼滤波
    gyro_accel_data.accel.accel_x = Com_Filter_KalmanFilter(&kfs[0], gyro_accel_data.accel.accel_x);
    gyro_accel_data.accel.accel_y = Com_Filter_KalmanFilter(&kfs[1], gyro_accel_data.accel.accel_y);
    gyro_accel_data.accel.accel_z = Com_Filter_KalmanFilter(&kfs[2], gyro_accel_data.accel.accel_z);
    // 打印加速度
    // debug_printf(":%d,%d,%d\n", gyro_accel_data.accel_x, gyro_accel_data.accel_y, gyro_accel_data.accel_z);

    // 4. 通过加速度和角速度来计算当前飞机倾斜的角度 => 姿态解算
    // // 4.1 使用互补解算计算欧拉角 => 优先使用加速度解算 => 俯仰角、横滚角能够使用
    // euler_angle.pitch = atan2(gyro_accel_data.accel.accel_y, gyro_accel_data.accel.accel_z) * 180 / 3.14159;
    // euler_angle.roll = atan2(gyro_accel_data.accel.accel_x, gyro_accel_data.accel.accel_z) * 180 / 3.14159;
    // // 偏航角 => 只能使用角速度积分 => 16位ADC的值转换为°/s => 量程是±2000
    // gyro_z_sum += (gyro_accel_data.gyro.gyro_z * 2000 / 32768.0f) * 0.006;
    // euler_angle.yaw = gyro_z_sum;
    // 4.2 使用移植的四元数姿态解算
    Com_Imu_GetEulerAngle(&gyro_accel_data, &euler_angle, 0.006);

    debug_printf(":%d,%d,%d\n", euler_angle.pitch, euler_angle.roll, euler_angle.yaw);
}

/**
 * @brief 根据欧拉角，计算出PID的目标值
 */
void App_flight_pid_process(void)
{
    // 俯仰角
    // 1. 需要赋值目标值、测量值 => 外环内环测量值的单位要保持一致
    // 外环的目标角度 => remote_data.pit（0~1000，500为中间点） => 控制范围在±10°
    pitch_outer_pid.target = (500 - remote_data.pit) / 50.0f;
    roll_outer_pid.target = (remote_data.rol - 500) / 50.0f;
    yaw_outer_pid.target = (500 - remote_data.yaw) / 50.0f;
    // 外环的测量值 => 当前的角度
    pitch_outer_pid.measure = euler_angle.pitch;
    roll_outer_pid.measure = euler_angle.roll;
    yaw_outer_pid.measure = euler_angle.yaw;
    // 内环的测量值 => 当前的角速度
    if (MPU_SELECT_DIR) // X轴为机头方向
    {
        pitch_inner_pid.measure = gyro_accel_data.gyro.gyro_y * 2000.0f / 32768.0f;
        roll_inner_pid.measure = gyro_accel_data.gyro.gyro_x * 2000.0f / 32768.0f;
        yaw_inner_pid.measure = gyro_accel_data.gyro.gyro_z * 2000.0f / 32768.0f;
    }
    else // Y轴为机头方向
    {
        pitch_inner_pid.measure = gyro_accel_data.gyro.gyro_x * 2000.0f / 32768.0f;
        roll_inner_pid.measure = gyro_accel_data.gyro.gyro_y * 2000.0f / 32768.0f;
        yaw_inner_pid.measure = gyro_accel_data.gyro.gyro_z * 2000.0f / 32768.0f;
    }

    // 2. 进行PID计算
    Com_PID_Calc_Chain(&pitch_outer_pid, &pitch_inner_pid);
    Com_PID_Calc_Chain(&roll_outer_pid, &roll_inner_pid);
    Com_PID_Calc_Chain(&yaw_outer_pid, &yaw_inner_pid);
}

/**
 * @brief 根据飞行状态控制电机
 *
 * @note 返航状态的控制逻辑暂未写出
 */
void App_flight_control_motor(void)
{
    // 1. 首先判断当前飞机的飞行状态
    switch (flight_state)
    {
    case IDLE:
        // 一旦进入加锁状态 => 需要将电机速度设置为0
        left_top_motor.speed = 0;
        left_bottom_motor.speed = 0;
        right_top_motor.speed = 0;
        right_bottom_motor.speed = 0;
        break;
    case NORMAL:
        // 俯仰角 => 左上右上一组，左下右下一组
        // 横滚角 => 左上左下一组，右上右下一组
        // 偏航角 => 左上右下一组，左下右上一组
        // 不同重要程度的PID控制结果，可以进行适当的限制
        left_top_motor.speed = remote_data.thr + pitch_inner_pid.output + roll_inner_pid.output + Com_limit(yaw_inner_pid.output, 120, -120);
        left_bottom_motor.speed = remote_data.thr - pitch_inner_pid.output + roll_inner_pid.output - Com_limit(yaw_inner_pid.output, 120, -120);
        right_top_motor.speed = remote_data.thr + pitch_inner_pid.output - roll_inner_pid.output - Com_limit(yaw_inner_pid.output, 120, -120);
        right_bottom_motor.speed = remote_data.thr - pitch_inner_pid.output - roll_inner_pid.output + Com_limit(yaw_inner_pid.output, 120, -120);
        break;
    case FIX_HEIGHT:
        // 只有定高状态才需要进行定高的PID计算 => 定高同时也需要平稳飞行
        left_top_motor.speed = HEIGHT_CONTROL_THRESHOLD + height_pid.output + pitch_inner_pid.output + roll_inner_pid.output + Com_limit(yaw_inner_pid.output, 120, -120);
        left_bottom_motor.speed = HEIGHT_CONTROL_THRESHOLD + height_pid.output - pitch_inner_pid.output + roll_inner_pid.output - Com_limit(yaw_inner_pid.output, 120, -120);
        right_top_motor.speed = HEIGHT_CONTROL_THRESHOLD + height_pid.output + pitch_inner_pid.output - roll_inner_pid.output - Com_limit(yaw_inner_pid.output, 120, -120);
        right_bottom_motor.speed = HEIGHT_CONTROL_THRESHOLD + height_pid.output - pitch_inner_pid.output - roll_inner_pid.output + Com_limit(yaw_inner_pid.output, 120, -120);
        break;
    case HEIGHT_CONTROL:
        // 高度调制模式 => 通过遥控器的油门来控制高度 => 油门越大 => 高度越高
        left_top_motor.speed = HEIGHT_CONTROL_THRESHOLD + height_pid.output + pitch_inner_pid.output + roll_inner_pid.output + Com_limit(yaw_inner_pid.output, 120, -120);
        left_bottom_motor.speed = HEIGHT_CONTROL_THRESHOLD + height_pid.output - pitch_inner_pid.output + roll_inner_pid.output - Com_limit(yaw_inner_pid.output, 120, -120);
        right_top_motor.speed = HEIGHT_CONTROL_THRESHOLD + height_pid.output + pitch_inner_pid.output - roll_inner_pid.output - Com_limit(yaw_inner_pid.output, 120, -120);
        right_bottom_motor.speed = HEIGHT_CONTROL_THRESHOLD + height_pid.output - pitch_inner_pid.output - roll_inner_pid.output + Com_limit(yaw_inner_pid.output, 120, -120);
        break;
    case FAIL:
        // 进行故障处理 => 一直处理直到满足条件 => 修改状态为IDLE
        // 6ms执行一次 => 速度-2
        left_top_motor.speed -= 2;
        left_bottom_motor.speed -= 2;
        right_top_motor.speed -= 2;
        right_bottom_motor.speed -= 2;

        if (left_top_motor.speed <= 0 && left_bottom_motor.speed <= 0 && right_top_motor.speed <= 0 && right_bottom_motor.speed <= 0)
        {
            // 故障处理完成 => 电机转速都已经降低为0
            xTaskNotifyGive(com_task_handle);
        }
        break;
    default:
        break;
    }

    // 2. 安全限制，限制电机速度的上限值
    if (remote_data.thr > 50)
    {
        // 可以通过提高速度上限 => 让飞行更平稳
        left_top_motor.speed = Com_limit(left_top_motor.speed, MOTOR_MAX_SPEED, 0);
        left_bottom_motor.speed = Com_limit(left_bottom_motor.speed, MOTOR_MAX_SPEED, 0);
        right_top_motor.speed = Com_limit(right_top_motor.speed, MOTOR_MAX_SPEED, 0);
        right_bottom_motor.speed = Com_limit(right_bottom_motor.speed, MOTOR_MAX_SPEED, 0);
    }
    else // 如果当前油门小于50，则强制将电机速度设置为0
    {
        left_top_motor.speed = 0;
        left_bottom_motor.speed = 0;
        right_top_motor.speed = 0;
        right_bottom_motor.speed = 0;
    }

    // 3. 设置电机速度
    Int_motor_set_speed(&left_top_motor);
    Int_motor_set_speed(&left_bottom_motor);
    Int_motor_set_speed(&right_top_motor);
    Int_motor_set_speed(&right_bottom_motor);
}

/**
 * @brief 进入定高功能之后的PID计算
 */
void App_flight_fix_height_process(void)
{
    // 24ms一次
    // 1. 获取当前高度
    // now_height = Int_VL53L1X_GetDistance();
    uint16_t temp = Int_VL53L0X_ReadContinuous();
    if (temp != 0)
    {
        now_height = temp;
        now_height = (float)now_height * 0.75f + last_height * 0.25f; // 一阶低通滤波，平滑高度值
        last_height = now_height;
    }

    // 2. 定高模式
    if (flight_state == FIX_HEIGHT)
    {
        // 3. 填写目标值和测量值
        height_pid.target = fix_height;
        height_pid.measure = now_height;

        // 4. 进行单环PID计算 => 得到输出值
        Com_PID_Calc(&height_pid);
    }
    // 3. 高度调制模式
    else if (flight_state == HEIGHT_CONTROL)
    {
        // 3. 填写目标值和测量值
        height_pid.target = (remote_data.thr - 50) * 1.5; // 油门的范围是0~950 => 高度调制模式的高度范围是0~1425
        if (remote_data.thr < 50)
            height_pid.target = 0;
        height_pid.measure = (int16_t)now_height - (int16_t)height_control;
        if (height_pid.measure < 0)
            height_pid.measure = 0;
        // 4. 进行单环PID计算 => 得到输出值
        Com_PID_Calc(&height_pid);
    }
}
