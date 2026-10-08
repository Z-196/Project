#ifndef INT_OLED_FONT_H
#define INT_OLED_FONT_H

#include <stdint.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdarg.h>

/*中文字符字节宽度*/
#define Int_OLED_CHN_CHAR_WIDTH			3		//UTF-8编码格式给3，GB2312编码格式给2

/*ASCII字模数据声明*/
extern const uint8_t Int_OLED_F8x16[][16];
extern const uint8_t Int_OLED_F6x8[][6];

#endif // INT_OLED_FONT_H

