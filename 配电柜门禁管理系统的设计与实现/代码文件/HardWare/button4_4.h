#ifndef __button4_4_h
#define __button4_4_h
#include <stdint.h>
#include <stdbool.h>
#include "stm32f10x.h"
#include "Delay.h"
#include <string.h>
#include "Store.h"
#include "oled.h"
#include "main.h"

// 行引脚配置（输入，上拉）
#define ROW_PORT1      GPIOA
#define ROW_PORT2      GPIOB
#define ROW1_PIN       GPIO_Pin_11
#define ROW2_PIN       GPIO_Pin_12
#define ROW3_PIN       GPIO_Pin_15
#define ROW4_PIN       GPIO_Pin_3
#define ROW1_2_3_PINS  (ROW1_PIN | ROW2_PIN | ROW3_PIN)
#define ROW4_PIN_DEF   ROW4_PIN
#define ROW_RCC        RCC_APB2Periph_GPIOA

// 列引脚配置（输出，推挽）
#define COL_PORT       GPIOB
#define COL1_PIN       GPIO_Pin_4
#define COL2_PIN       GPIO_Pin_5
#define COL3_PIN       GPIO_Pin_6
#define COL4_PIN       GPIO_Pin_7
#define COL_PINS       (COL1_PIN | COL2_PIN | COL3_PIN | COL4_PIN)
#define COL_RCC        RCC_APB2Periph_GPIOB

// 密码系统配置
#define PASSWORD_MAX_COUNT     3     // 最大存储密码数+1	//实际为4个
#define PASSWORD_LENGTH        4     // 密码长度
#define PASSWORD_CONFIRM_KEY   11    // 确认键
#define PASSWORD_DELETE_KEY    12    // 删除键
#define PASSWORD_DEL_MODE_KEY  13    // 删除模式键
#define KEY_NONE               0xFF  // 无按键

// 定义密码在Store_Data数组中的起始索引
#define STORE_PASSWORD_START_INDEX	2  // 选择一个未使用的位置	//需留出四位管理员密码
#define STORE_PASSWORD_COUNT_INDEX  1  // 存储密码数量的位置	//禁止使用第零位

// 密码类型定义
typedef enum {
    PASSWORD_TYPE_ADMIN = 0,         // 管理员密码
    PASSWORD_TYPE_USER = 1           // 用户密码
} PasswordType;

// 密码操作结果
typedef enum {
    PASSWORD_SUCCESS = 0,            // 操作成功
    PASSWORD_ERROR_DUPLICATE,        // 密码重复
    PASSWORD_ERROR_STORAGE_FULL,     // 存储已满
    PASSWORD_ERROR_INVALID_PARAM,    // 参数无效
    PASSWORD_ERROR_PERMISSION_DENIED // 权限不足
} PasswordResult;
 
 // 系统状态
typedef enum {
	STATE_LOGIN = 0,
  STATE_BUTTON_LOGIN,
	STATE_BUTTON_MENU,
	STATE_AS608_LOGIN,
	STATE_AS608_MENU,
	STATE_AS608_LIS,
	STATE_AS608_ADD,
	STATE_AS608_DEL,
	STATE_RFID_LOGIN,
	STATE_RFID_MENU,
	STATE_RFID_LIS,
	STATE_RFID_ADD,
	STATE_RFID_DEL,
  STATE_MENU,
  STATE_ADD_PASSWORD,
  STATE_DELETE_PASSWORD,
	STATE_USER_MENU,
	STATE_AS608_ADMIN,
} SystemState;

extern bool admin_mode;
// 矩阵键盘状态枚举
typedef enum {
    KEY_STATE_RELEASED = 0,  // 按键释放状态
    KEY_STATE_PRESSED,       // 按键按下状态
    KEY_STATE_CONFIRMED      // 按键确认状态
} KeyState;
// 密码存储结构
typedef struct {
    uint8_t passwords[PASSWORD_MAX_COUNT][PASSWORD_LENGTH];  // 存储的密码
    uint8_t password_count;                                  // 已存密码数量
    uint8_t input_buffer[PASSWORD_LENGTH];                   // 输入缓冲区
    uint8_t input_index;                                     // 输入位置索引
    bool show_real_password;                                 // 显示模式：true-显示真实密码 false-显示*
    bool delete_mode;                                        // 删除模式
    uint8_t selected_password;                               // 当前选择的密码序号
} PasswordPanelContext;

extern PasswordPanelContext ctx;
extern SystemState current_state;
extern uint8_t key_value;

extern uint8_t dynamic_password[PASSWORD_LENGTH];	//动态密码存放数组
extern uint8_t dynamic_password_valid;				//0表示未设置动态密码

void Keyboard_GPIO_Config(void);		//初始化 矩阵按键的I/O口

uint16_t keyboard_scan(void);// 扫描按键并返回按键值

// 初始化密码系统
void PasswordPanel_Init(void);

// 获取输入缓冲区
const uint8_t* PasswordPanel_GetInputBuffer(void);

// 获取当前输入索引
uint8_t PasswordPanel_GetInputIndex(void);

// 处理按键输入
void PasswordPanel_HandleKey(uint8_t key);

// 验证当前输入的密码
PasswordResult PasswordPanel_VerifyPassword(bool *is_admin);

// 添加新密码
PasswordResult PasswordPanel_AddPassword(const uint8_t *password, PasswordType type);

// 删除密码
PasswordResult PasswordPanel_DeletePassword(uint8_t index);

// 获取密码数量
uint8_t PasswordPanel_GetPasswordCount(void);

// 获取指定索引的密码
const uint8_t* PasswordPanel_GetPassword(uint8_t index);

// 清空输入缓冲区
void PasswordPanel_ClearInput(void);

// 设置显示模式（显示真实密码或*）
void PasswordPanel_SetShowMode(bool show_real);

// 获取当前显示模式
bool PasswordPanel_GetShowMode(void);

// 进入/退出删除模式
void PasswordPanel_SetDeleteMode(bool enable);

// 获取当前删除模式状态
bool PasswordPanel_GetDeleteMode(void);

// 设置当前选中的密码索引
void PasswordPanel_SetSelectedPassword(uint8_t index);

// 获取当前选中的密码索引
uint8_t PasswordPanel_GetSelectedPassword(void);

// 绘制密码输入界面	// 登录界面
void PasswordPanel_DrawInputUI(void);

// 处理菜单状态	// 管理员菜单
void handleMenuState(void);

// 处理菜单状态	// 用户菜单
void userleMenuState(void);
	
// 绘制密码列表界面
void PasswordPanel_DrawPasswordListUI(void);

// 绘制添加密码界面
void PasswordPanel_DrawAddPasswordUI(void);

// 处理删除密码状态
void handleDeletePasswordState(void);

// 绘制操作结果提示
void PasswordPanel_DrawResultMessage(const char *message);
 
// 保存所有密码到Store_Data并调用Store_Save
void PasswordPanel_SavePasswords(void);

// 动态密码功能
#define DYNAMIC_PASSWORD_VALID_TIME_MS  120000  // 动态密码有效期：2分钟（120秒 = 120000毫秒）

// 设置动态密码（由APP调用）
void PasswordPanel_SetDynamicPassword(const uint8_t *password);

// 验证动态密码
PasswordResult PasswordPanel_VerifyDynamicPassword(const uint8_t *password);

// 检查动态密码是否有效
uint8_t PasswordPanel_IsDynamicPasswordValid(void);
	
#endif
