#include "button4_4.h"

bool admin_mode = false;

uint8_t flag = 0;
uint8_t key_value = KEY_NONE;

const uint8_t key_map[4][4] = {
    {1,  2,  3,  4},
    {5,  6,  7,  8},
    {9, 10, 11, 12},
    {13,14,15, 0}
};

static KeyState key_state = KEY_STATE_RELEASED;
static uint8_t last_key_index = 0xFF;
PasswordPanelContext ctx;
SystemState current_state = STATE_LOGIN;

uint8_t dynamic_password[PASSWORD_LENGTH] = {0};
uint32_t dynamic_password_time = 0;
uint8_t dynamic_password_valid = 0;	//0表示未设置动态密码

void PasswordPanel_Init(void) {
    memset(&ctx, 0, sizeof(ctx));
    ctx.password_count = 2; // 固定 2 组：管理员+用户	

    // 默认：管理员密码 0000
    ctx.passwords[0][0] = g_Params.passwords[0][0];
    ctx.passwords[0][1] = g_Params.passwords[0][1];
    ctx.passwords[0][2] = g_Params.passwords[0][2];
    ctx.passwords[0][3] = g_Params.passwords[0][3];

    // 默认：用户密码 1234
    ctx.passwords[1][0] = g_Params.passwords[1][0];
    ctx.passwords[1][1] = g_Params.passwords[1][1];
    ctx.passwords[1][2] = g_Params.passwords[1][2];
    ctx.passwords[1][3] = g_Params.passwords[1][3];		
}

void Keyboard_GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    // 必须先开时钟
    RCC_APB2PeriphClockCmd(ROW_RCC | COL_RCC | RCC_APB2Periph_AFIO, ENABLE);
    
    // 关闭 JTAG，释放 PA15 作为普通IO（关键！）
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;

    // 列配置 -> 推挽输出
    GPIO_InitStruct.GPIO_Pin = COL_PINS;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(COL_PORT, &GPIO_InitStruct);

    // 行1 2 3 -> PA11/12/15 上拉输入
    GPIO_InitStruct.GPIO_Pin = ROW1_2_3_PINS;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(ROW_PORT1, &GPIO_InitStruct);

    // 行4 -> PB3 上拉输入
    GPIO_InitStruct.GPIO_Pin = ROW4_PIN_DEF;
    GPIO_Init(ROW_PORT2, &GPIO_InitStruct);

    // 默认所有列输出高电平
    COL_PORT->ODR |= COL_PINS;
    
    PasswordPanel_Init();
}

// 扫描矩阵键盘（返回原始按键索引 0~15，无按键返回 0xFF）
static uint8_t scan_raw_key(void)
{
    uint8_t col;
    
    // 逐列扫描 COL1~COL4：PB4~PB7
    for (col = 0; col < 4; col++) {
        // 所有列先拉高
        COL_PORT->ODR |= COL_PINS;
        
        // 拉低当前选中列
        switch(col) {
            case 0: COL_PORT->ODR &= ~COL1_PIN; break;
            case 1: COL_PORT->ODR &= ~COL2_PIN; break;
            case 2: COL_PORT->ODR &= ~COL3_PIN; break;
            case 3: COL_PORT->ODR &= ~COL4_PIN; break;
        }
        
        Delay_us(10);
        
        // 分别读取两个端口的行状态
        uint32_t rowA = ROW_PORT1->IDR; // PA
        uint32_t rowB = ROW_PORT2->IDR; // PB
        
        // 判断是否有按键按下
        if( ((rowA & ROW1_2_3_PINS) != ROW1_2_3_PINS) || ((rowB & ROW4_PIN) == 0) )
        {
            Delay_ms(10); // 消抖
            
            // 再次读取确认
            rowA = ROW_PORT1->IDR;
            rowB = ROW_PORT2->IDR;
            
            // 严格顺序判断：行1 → 行2 → 行3 → 行4
            if ((rowA & ROW1_PIN) == 0)  return col * 4 + 0;   // 行1
            if ((rowA & ROW2_PIN) == 0)  return col * 4 + 1;   // 行2
            if ((rowA & ROW3_PIN) == 0)  return col * 4 + 2;   // 行3 (PA15)
            if ((rowB & ROW4_PIN) == 0)  return col * 4 + 3;   // 行4 (PB3)
        }
    }
    
    // 扫描完成，所有列恢复高电平
    COL_PORT->ODR |= COL_PINS;
    
    return 0xFF; // 无按键
}

//static uint8_t scan_raw_key(void)
//{
//    uint8_t col;
//    uint16_t col_val;

//    // 全部输出高
//    COL_PORT->ODR |= COL_PINS;

//    // 逐列拉低
//    for(col=0; col<4; col++){
//        // 拉低当前列
//        COL_PORT->ODR &= ~(1 << (4+col));
//        Delay_us(10);

//        col_val = ROW_PORT1->IDR;
//        if( !(col_val & ROW1_PIN) ){ Delay_ms(10); if( !(ROW_PORT1->IDR & ROW1_PIN) ){ COL_PORT->ODR |= COL_PINS; return 0*4 + col; } }
//        if( !(col_val & ROW2_PIN) ){ Delay_ms(10); if( !(ROW_PORT1->IDR & ROW2_PIN) ){ COL_PORT->ODR |= COL_PINS; return 1*4 + col; } }
//        if( !(col_val & ROW3_PIN) ){ Delay_ms(10); if( !(ROW_PORT1->IDR & ROW3_PIN) ){ COL_PORT->ODR |= COL_PINS; return 2*4 + col; } }

//        col_val = ROW_PORT2->IDR;
//        if( !(col_val & ROW4_PIN) ){ Delay_ms(10); if( !(ROW_PORT2->IDR & ROW4_PIN) ){ COL_PORT->ODR |= COL_PINS; return 3*4 + col; } }

//        COL_PORT->ODR |= (1 << (4+col));
//    }

//    COL_PORT->ODR |= COL_PINS;
//    return 0xFF;
//}

uint16_t keyboard_scan(void)
{
    uint8_t key_index = scan_raw_key();
    uint16_t key_val = KEY_NONE;

    switch (key_state) {
        case KEY_STATE_RELEASED:
            if (key_index != 0xFF) {
                last_key_index = key_index;
                key_state = KEY_STATE_PRESSED;
            }
            break;

        case KEY_STATE_PRESSED:
            if (key_index == last_key_index) {
                key_state = KEY_STATE_CONFIRMED;
                key_val = key_map[last_key_index / 4][last_key_index % 4];
            } else {
                key_state = KEY_STATE_RELEASED;
            }
            break;

        case KEY_STATE_CONFIRMED:
            if (key_index == 0xFF) {
                key_state = KEY_STATE_RELEASED;
            }
            break;
    }
    return key_val;
}

uint8_t PasswordPanel_GetInputIndex(void) {
    return ctx.input_index;
}

// 处理按键输入
void PasswordPanel_HandleKey(uint8_t key) {
    if(key == KEY_NONE) return;

    if(key == PASSWORD_DELETE_KEY) {
        if(ctx.input_index > 0) {
            ctx.input_index--;
            ctx.input_buffer[ctx.input_index] = 0;
        }
        return;
    }

    if(key == PASSWORD_CONFIRM_KEY) return;

    if(ctx.input_index < PASSWORD_LENGTH && key <= 9) {
        ctx.input_buffer[ctx.input_index] = key;
        ctx.input_index++;
    }
}

PasswordResult PasswordPanel_VerifyPassword(bool *is_admin) {
    if(ctx.input_index != PASSWORD_LENGTH) {
        return PASSWORD_ERROR_INVALID_PARAM;
    }

    bool valid = false;
    *is_admin = false;

    // 首先检查动态密码（优先级最高）
    if(dynamic_password_valid && PasswordPanel_IsDynamicPasswordValid()) {
        if(memcmp(ctx.input_buffer, dynamic_password, PASSWORD_LENGTH) == 0) {
            valid = true;
            *is_admin = false;  // 动态密码作为用户密码
            // 清空输入缓冲
            PasswordPanel_ClearInput();
            return PASSWORD_SUCCESS;
        }
    }	
	
    for(int i=0; i<ctx.password_count; i++) {
        if(memcmp(ctx.input_buffer, ctx.passwords[i], 4) == 0) {
            valid = true;
            if(i==0) *is_admin = true;
            break;
        }
    }

    PasswordPanel_ClearInput();
    return valid ? PASSWORD_SUCCESS : PASSWORD_ERROR_INVALID_PARAM;
}

PasswordResult PasswordPanel_AddPassword(const uint8_t *password, PasswordType type) // 0管理员 1用户
{ 
	if(!password)  return PASSWORD_ERROR_INVALID_PARAM;
	
    // 只修改内存，不写Flash
    memcpy(ctx.passwords[type], password, PASSWORD_LENGTH);
    return PASSWORD_SUCCESS;
}

const uint8_t* PasswordPanel_GetInputBuffer(void) {
    return ctx.input_buffer;
}

void PasswordPanel_ClearInput(void) {
    memset(ctx.input_buffer, 0, 4);
    ctx.input_index = 0;
}

// ====================== 明文显示密码 ======================
void PasswordPanel_DrawInputUI(void) {
    OLED_Clear();
    OLED_ShowChinese(0, 16, "密码验证");
    OLED_ShowString(0, 39, "PW:", OLED_6X8);

    for(int i=0; i<4; i++) {
        if(i < ctx.input_index)
            OLED_ShowNum(24+i*6, 39, ctx.input_buffer[i], 1, OLED_6X8);
        else
            OLED_ShowChar(24+i*6, 39, '_', OLED_6X8);
    }

    OLED_Printf(0,64,OLED_8X16,"S12删除 S3确认");
    OLED_Update();
}

void PasswordPanel_DrawAddPasswordUI(void) {}
void PasswordPanel_DrawPasswordListUI(void) {}
void handleDeletePasswordState(void) {}
void PasswordPanel_DrawResultMessage(const char *message) {}
void PasswordPanel_SavePasswords(void) {
//    Store_Data[1] = ctx.password_count;
//    Store_Data[2] = ctx.passwords[0][0];
//    Store_Data[3] = ctx.passwords[0][1];
//    Store_Data[4] = ctx.passwords[0][2];
//    Store_Data[5] = ctx.passwords[0][3];
//    Store_Save();
}

// 设置动态密码（由APP调用）
void PasswordPanel_SetDynamicPassword(const uint8_t *password) {
    if(password != NULL) {
        memcpy(dynamic_password, password, PASSWORD_LENGTH);
    }
}

// 验证动态密码
PasswordResult PasswordPanel_VerifyDynamicPassword(const uint8_t *password) {
    if(password == NULL) {
        return PASSWORD_ERROR_INVALID_PARAM;
    }
    
    // 检查动态密码是否有效
    if(!dynamic_password_valid || !PasswordPanel_IsDynamicPasswordValid()) {
        return PASSWORD_ERROR_INVALID_PARAM;  // 动态密码已过期
    }
    
    // 比较密码
    if(memcmp(password, dynamic_password, PASSWORD_LENGTH) == 0) {
        return PASSWORD_SUCCESS;
    }
    
    return PASSWORD_ERROR_INVALID_PARAM;
}

// 检查动态密码是否有效（2分钟内）
uint8_t PasswordPanel_IsDynamicPasswordValid(void) {
    if(!dynamic_password_valid) {
        return 0;  // 未设置动态密码
    }
    
}
