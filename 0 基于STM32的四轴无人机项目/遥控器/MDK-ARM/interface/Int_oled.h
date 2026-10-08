#ifndef INT_Int_OLED_H
#define INT_Int_OLED_H

#include "i2c.h"
#include "Int_oled_font.h"

/*FontSize参数取值*/
/*此参数值不仅用于判断，而且用于计算横向字符偏移，默认值为字体像素宽度*/
#define Int_OLED_8X16				8
#define Int_OLED_6X8				6

/*IsFilled参数数值*/
#define Int_OLED_UNFILLED			0
#define Int_OLED_FILLED				1


/*初始化函数*/
void Int_OLED_Init(void);

/*更新函数*/
void Int_OLED_Update(void);
void Int_OLED_UpdateArea(uint8_t X, uint8_t Y, uint8_t Width, uint8_t Height);

/*显存控制函数*/
void Int_OLED_Clear(void);
void Int_OLED_ClearArea(uint8_t X, uint8_t Y, uint8_t Width, uint8_t Height);
void Int_OLED_Reverse(void);
void Int_OLED_ReverseArea(uint8_t X, uint8_t Y, uint8_t Width, uint8_t Height);

/*显示函数*/
void Int_OLED_ShowChar(uint8_t X, uint8_t Y, char Char, uint8_t FontSize);
void Int_OLED_ShowString(uint8_t X, uint8_t Y, char *String, uint8_t FontSize);
void Int_OLED_ShowNum(uint8_t X, uint8_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize);
void Int_OLED_ShowSignedNum(uint8_t X, uint8_t Y, int32_t Number, uint8_t Length, uint8_t FontSize);
void Int_OLED_ShowHexNum(uint8_t X, uint8_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize);
void Int_OLED_ShowBinNum(uint8_t X, uint8_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize);
void Int_OLED_ShowFloatNum(uint8_t X, uint8_t Y, double Number, uint8_t IntLength, uint8_t FraLength, uint8_t FontSize);
void Int_OLED_ShowImage(uint8_t X, uint8_t Y, uint8_t Width, uint8_t Height, const uint8_t *Image);
void Int_OLED_Printf(uint8_t X, uint8_t Y, uint8_t FontSize, char *format, ...);

/*绘图函数*/
void Int_OLED_DrawPoint(uint8_t X, uint8_t Y);
uint8_t Int_OLED_GetPoint(uint8_t X, uint8_t Y);
void Int_OLED_DrawLine(uint8_t X0, uint8_t Y0, uint8_t X1, uint8_t Y1);
void Int_OLED_DrawRectangle(uint8_t X, uint8_t Y, uint8_t Width, uint8_t Height, uint8_t IsFilled);
void Int_OLED_DrawTriangle(uint8_t X0, uint8_t Y0, uint8_t X1, uint8_t Y1, uint8_t X2, uint8_t Y2, uint8_t IsFilled);
void Int_OLED_DrawCircle(uint8_t X, uint8_t Y, uint8_t Radius, uint8_t IsFilled);
void Int_OLED_DrawEllipse(uint8_t X, uint8_t Y, uint8_t A, uint8_t B, uint8_t IsFilled);
void Int_OLED_DrawArc(uint8_t X, uint8_t Y, uint8_t Radius, int16_t StartAngle, int16_t EndAngle, uint8_t IsFilled);

#endif // INT_Int_OLED_H
