#ifndef _AS608_use_h_
#define _AS608_use_h_
#include "stm32f10x.h"   //stm32头文件
#include "as608.h"						/*指纹模块初始化*/
#include "DELAY.h"						/*延时函数*/
#define MAX_FINGERPRINTS 4     // 最大存储指纹数量
#define MAX_DATA_LENGTH 50      // 每个指纹关联数据最大长度
#define usart2_baund  57600			/*串口2波特率，根据指纹模块波特率更改*/

/**
  * @brief  指纹信息结构体
  * @note   用于存储指纹ID与关联信息的映射关系
  */
typedef struct {
    u8 fingerprint_id;          // 指纹ID（对应AS608模块中的页码）
    u8 valid;                   // 有效标志位 0:无效 1:有效
	u8 sing;
} FingerprintInfo;

extern FingerprintInfo fingerprintDB[MAX_FINGERPRINTS]; // 指纹信息数据库

u8 Add_FR(u16 FR_ID);						/*录哪个ID的指纹，返回1=成功，0=用户取消*/
void Del_FR(u16 FR_ID);						/*删除哪个ID的指纹*/
void press_FR(void);							/*刷指纹*/
void ShowErrMessage(u8 ensure);		/*显示确认码错误信息*/
void AS608_Init(void);
void delate_fingerprints(void);//删指纹函数;
void record_fingerprints(void);//录指纹函数
void ListAllFingerprints(void);//列出所有有效指纹信息
void DeleteFromFingerprintDB(u8 fid, uint32_t icid);//从数据库删除指纹信息
u8 SaveToFingerprintDB(u8 fid);//保存指纹数据到数据库
void Input_Fingerprints(void);//录入指纹111111
u8 identify(void);//指纹识别
void del_Fingerprints(void);//指纹删除
void del_all_Fingerprints(void);//删除全部指纹
void verification_Fingerprint(void);//检测对比指纹
u8 GetAllValidFingerprintIDs(u8 *ids);//获取指纹模块中所有有效指纹的ID列表
void LoadFingerprintDBFromFlash(void);// 从 FLASH 加载指纹数据库
void UpdateFingerprintDBToFlash(void);// 将指纹数据库更新到 FLASH

#endif
