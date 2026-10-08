#include "Int_oled.h"

/**
 * Int_OLED显存数组
 * 所有的显示函数，都只是对此显存数组进行读写
 * 随后调用Int_OLED_Update函数或Int_OLED_UpdateArea函数
 * 才会将显存数组的数据发送到Int_OLED硬件，进行显示
 */
uint8_t Int_OLED_DisplayBuf[8][128];

static void Int_OLED_WriteBytes(uint8_t *Data, uint8_t Count)
{
    HAL_I2C_Master_Transmit(&hi2c1, 0x78, Data, Count, 200);
}

/**
 * 函    数：Int_OLED写命令
 * 参    数：Command 要写入的命令值，范围：0x00~0xFF
 * 返 回 值：无
 */
void Int_OLED_WriteCommand(uint8_t Command)
{
    uint8_t buf_data[2] = {0x00, Command};
    Int_OLED_WriteBytes(buf_data, 2);
}
/**
 * 函    数：Int_OLED写数据
 * 参    数：Data 要写入数据的起始地址
 * 参    数：Count 要写入数据的数量
 * 返 回 值：无
 */
void Int_OLED_WriteData(uint8_t *Data, uint8_t Count)
{
    uint8_t buf_data[129];
    buf_data[0] = 0x40;
    memcpy(&buf_data[1], Data, Count);
    Int_OLED_WriteBytes(buf_data, Count + 1);
}

/**
 * 函    数：Int_OLED初始化
 * 参    数：无
 * 返 回 值：无
 * 说    明：使用前，需要调用此初始化函数
 */
void Int_OLED_Init(void)
{
    /* 写入一系列的命令，对Int_OLED进行初始化配置 */
    Int_OLED_WriteCommand(0xAE); // 设置显示开启/关闭，0xAE关闭，0xAF开启

    Int_OLED_WriteCommand(0xD5); // 设置显示时钟分频比/振荡器频率
    Int_OLED_WriteCommand(0x80); // 0x00~0xFF

    Int_OLED_WriteCommand(0xA8); // 设置多路复用率
    Int_OLED_WriteCommand(0x3F); // 0x0E~0x3F

    Int_OLED_WriteCommand(0xD3); // 设置显示偏移
    Int_OLED_WriteCommand(0x00); // 0x00~0x7F

    Int_OLED_WriteCommand(0x40); // 设置显示开始行，0x40~0x7F

    Int_OLED_WriteCommand(0xA1); // 设置左右方向，0xA1正常，0xA0左右反置

    Int_OLED_WriteCommand(0xC8); // 设置上下方向，0xC8正常，0xC0上下反置

    Int_OLED_WriteCommand(0xDA); // 设置COM引脚硬件配置
    Int_OLED_WriteCommand(0x12);

    Int_OLED_WriteCommand(0x81); // 设置对比度
    Int_OLED_WriteCommand(0xCF); // 0x00~0xFF

    Int_OLED_WriteCommand(0xD9); // 设置预充电周期
    Int_OLED_WriteCommand(0xF1);

    Int_OLED_WriteCommand(0xDB); // 设置VCOMH取消选择级别
    Int_OLED_WriteCommand(0x30);

    Int_OLED_WriteCommand(0xA4); // 设置整个显示打开/关闭

    Int_OLED_WriteCommand(0xA6); // 设置正常/反色显示，0xA6正常，0xA7反色

    Int_OLED_WriteCommand(0x8D); // 设置充电泵
    Int_OLED_WriteCommand(0x14);

    Int_OLED_WriteCommand(0xAF); // 开启显示
}

/**
 * 函    数：Int_OLED设置显示光标位置
 * 参    数：Page 指定光标所在的页，范围：0~7
 * 参    数：X 指定光标所在的X轴坐标，范围：0~127
 * 返 回 值：无
 * 说    明：Int_OLED默认的Y轴，只能8个Bit为一组写入，即1页等于8个Y轴坐标
 */
void Int_OLED_SetCursor(uint8_t Page, uint8_t X)
{
    /*通过指令设置页地址和列地址*/
    Int_OLED_WriteCommand(0xB0 | Page);              // 设置页位置
    Int_OLED_WriteCommand(0x10 | ((X & 0xF0) >> 4)); // 设置X位置高4位
    Int_OLED_WriteCommand(0x00 | (X & 0x0F));        // 设置X位置低4位
}

/**
 * 函    数：次方函数
 * 参    数：X 底数
 * 参    数：Y 指数
 * 返 回 值：等于X的Y次方
 */
uint32_t Int_OLED_Pow(uint32_t X, uint32_t Y)
{
    uint32_t Result = 1; // 结果默认为1
    while (Y--)          // 累乘Y次
    {
        Result *= X; // 每次把X累乘到结果上
    }
    return Result;
}

/**
 * 函    数：判断指定点是否在指定多边形内部
 * 参    数：nvert 多边形的顶点数
 * 参    数：vertx verty 包含多边形顶点的x和y坐标的数组
 * 参    数：testx testy 测试点的X和y坐标
 * 返 回 值：指定点是否在指定多边形内部，1：在内部，0：不在内部
 */
uint8_t Int_OLED_pnpoly(uint8_t nvert, int16_t *vertx, int16_t *verty, int16_t testx, int16_t testy)
{
    int16_t i, j, c = 0;

    for (i = 0, j = nvert - 1; i < nvert; j = i++)
    {
        if (((verty[i] > testy) != (verty[j] > testy)) &&
            (testx < (vertx[j] - vertx[i]) * (testy - verty[i]) / (verty[j] - verty[i]) + vertx[i]))
        {
            c = !c;
        }
    }
    return c;
}

/**
 * 函    数：判断指定点是否在指定角度内部
 * 参    数：X Y 指定点的坐标
 * 参    数：StartAngle EndAngle 起始角度和终止角度，范围：-180~180
 *           水平向右为0度，水平向左为180度或-180度，下方为正数，上方为负数，顺时针旋转
 * 返 回 值：指定点是否在指定角度内部，1：在内部，0：不在内部
 */
uint8_t Int_OLED_IsInAngle(int16_t X, int16_t Y, int16_t StartAngle, int16_t EndAngle)
{
    int16_t PointAngle;
    PointAngle = atan2(Y, X) / 3.14 * 180; // 计算指定点的弧度，并转换为角度表示
    if (StartAngle < EndAngle)             // 起始角度小于终止角度的情况
    {
        /*如果指定角度在起始终止角度之间，则判定指定点在指定角度*/
        if (PointAngle >= StartAngle && PointAngle <= EndAngle)
        {
            return 1;
        }
    }
    else // 起始角度大于于终止角度的情况
    {
        /*如果指定角度大于起始角度或者小于终止角度，则判定指定点在指定角度*/
        if (PointAngle >= StartAngle || PointAngle <= EndAngle)
        {
            return 1;
        }
    }
    return 0; // 不满足以上条件，则判断判定指定点不在指定角度
}

/**
 * 函    数：将Int_OLED显存数组更新到Int_OLED屏幕
 * 参    数：无
 * 返 回 值：无
 * 说    明：所有的显示函数，都只是对Int_OLED显存数组进行读写
 *           随后调用Int_OLED_Update函数或Int_OLED_UpdateArea函数
 *           才会将显存数组的数据发送到Int_OLED硬件，进行显示
 *           故调用显示函数后，要想真正地呈现在屏幕上，还需调用更新函数
 */
void Int_OLED_Update(void)
{
    uint8_t j;
    /*遍历每一页*/
    for (j = 0; j < 8; j++)
    {
        /*设置光标位置为每一页的第一列*/
        Int_OLED_SetCursor(j, 0);
        /*连续写入128个数据，将显存数组的数据写入到Int_OLED硬件*/
        Int_OLED_WriteData(Int_OLED_DisplayBuf[j], 128);
    }
}

/**
 * 函    数：将Int_OLED显存数组部分更新到Int_OLED屏幕
 * 参    数：X 指定区域左上角的横坐标，范围：0~127
 * 参    数：Y 指定区域左上角的纵坐标，范围：0~63
 * 参    数：Width 指定区域的宽度，范围：0~128
 * 参    数：Height 指定区域的高度，范围：0~64
 * 返 回 值：无
 * 说    明：此函数会至少更新参数指定的区域
 *           如果更新区域Y轴只包含部分页，则同一页的剩余部分会跟随一起更新
 * 说    明：所有的显示函数，都只是对Int_OLED显存数组进行读写
 *           随后调用Int_OLED_Update函数或Int_OLED_UpdateArea函数
 *           才会将显存数组的数据发送到Int_OLED硬件，进行显示
 *           故调用显示函数后，要想真正地呈现在屏幕上，还需调用更新函数
 */
void Int_OLED_UpdateArea(uint8_t X, uint8_t Y, uint8_t Width, uint8_t Height)
{
    uint8_t j;

    /*参数检查，保证指定区域不会超出屏幕范围*/
    if (X > 127)
    {
        return;
    }
    if (Y > 63)
    {
        return;
    }
    if (X + Width > 128)
    {
        Width = 128 - X;
    }
    if (Y + Height > 64)
    {
        Height = 64 - Y;
    }

    /*遍历指定区域涉及的相关页*/
    /*(Y + Height - 1) / 8 + 1的目的是(Y + Height) / 8并向上取整*/
    for (j = Y / 8; j < (Y + Height - 1) / 8 + 1; j++)
    {
        /*设置光标位置为相关页的指定列*/
        Int_OLED_SetCursor(j, X);
        /*连续写入Width个数据，将显存数组的数据写入到Int_OLED硬件*/
        Int_OLED_WriteData(&Int_OLED_DisplayBuf[j][X], Width);
    }
}

/**
 * 函    数：将Int_OLED显存数组全部清零
 * 参    数：无
 * 返 回 值：无
 * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
 */
void Int_OLED_Clear(void)
{
    uint8_t i, j;
    for (j = 0; j < 8; j++) // 遍历8页
    {
        for (i = 0; i < 128; i++) // 遍历128列
        {
            Int_OLED_DisplayBuf[j][i] = 0x00; // 将显存数组数据全部清零
        }
    }
}

/**
 * 函    数：将Int_OLED显存数组部分清零
 * 参    数：X 指定区域左上角的横坐标，范围：0~127
 * 参    数：Y 指定区域左上角的纵坐标，范围：0~63
 * 参    数：Width 指定区域的宽度，范围：0~128
 * 参    数：Height 指定区域的高度，范围：0~64
 * 返 回 值：无
 * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
 */
void Int_OLED_ClearArea(uint8_t X, uint8_t Y, uint8_t Width, uint8_t Height)
{
    uint8_t i, j;

    /*参数检查，保证指定区域不会超出屏幕范围*/
    if (X > 127)
    {
        return;
    }
    if (Y > 63)
    {
        return;
    }
    if (X + Width > 128)
    {
        Width = 128 - X;
    }
    if (Y + Height > 64)
    {
        Height = 64 - Y;
    }

    for (j = Y; j < Y + Height; j++) // 遍历指定页
    {
        for (i = X; i < X + Width; i++) // 遍历指定列
        {
            Int_OLED_DisplayBuf[j / 8][i] &= ~(0x01 << (j % 8)); // 将显存数组指定数据清零
        }
    }
}

/**
 * 函    数：将Int_OLED显存数组全部取反
 * 参    数：无
 * 返 回 值：无
 * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
 */
void Int_OLED_Reverse(void)
{
    uint8_t i, j;
    for (j = 0; j < 8; j++) // 遍历8页
    {
        for (i = 0; i < 128; i++) // 遍历128列
        {
            Int_OLED_DisplayBuf[j][i] ^= 0xFF; // 将显存数组数据全部取反
        }
    }
}

/**
 * 函    数：将Int_OLED显存数组部分取反
 * 参    数：X 指定区域左上角的横坐标，范围：0~127
 * 参    数：Y 指定区域左上角的纵坐标，范围：0~63
 * 参    数：Width 指定区域的宽度，范围：0~128
 * 参    数：Height 指定区域的高度，范围：0~64
 * 返 回 值：无
 * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
 */
void Int_OLED_ReverseArea(uint8_t X, uint8_t Y, uint8_t Width, uint8_t Height)
{
    uint8_t i, j;

    /*参数检查，保证指定区域不会超出屏幕范围*/
    if (X > 127)
    {
        return;
    }
    if (Y > 63)
    {
        return;
    }
    if (X + Width > 128)
    {
        Width = 128 - X;
    }
    if (Y + Height > 64)
    {
        Height = 64 - Y;
    }

    for (j = Y; j < Y + Height; j++) // 遍历指定页
    {
        for (i = X; i < X + Width; i++) // 遍历指定列
        {
            Int_OLED_DisplayBuf[j / 8][i] ^= 0x01 << (j % 8); // 将显存数组指定数据取反
        }
    }
}

/**
 * 函    数：Int_OLED显示一个字符
 * 参    数：X 指定字符左上角的横坐标，范围：0~127
 * 参    数：Y 指定字符左上角的纵坐标，范围：0~63
 * 参    数：Char 指定要显示的字符，范围：ASCII码可见字符
 * 参    数：FontSize 指定字体大小
 *           范围：Int_OLED_8X16		宽8像素，高16像素
 *                 Int_OLED_6X8		宽6像素，高8像素
 * 返 回 值：无
 * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
 */
void Int_OLED_ShowChar(uint8_t X, uint8_t Y, char Char, uint8_t FontSize)
{
    if (FontSize == Int_OLED_8X16) // 字体为宽8像素，高16像素
    {
        /*将ASCII字模库Int_OLED_F8x16的指定数据以8*16的图像格式显示*/
        Int_OLED_ShowImage(X, Y, 8, 16, Int_OLED_F8x16[Char - ' ']);
    }
    else if (FontSize == Int_OLED_6X8) // 字体为宽6像素，高8像素
    {
        /*将ASCII字模库Int_OLED_F6x8的指定数据以6*8的图像格式显示*/
        Int_OLED_ShowImage(X, Y, 6, 8, Int_OLED_F6x8[Char - ' ']);
    }
}

/**
 * 函    数：Int_OLED显示字符串
 * 参    数：X 指定字符串左上角的横坐标，范围：0~127
 * 参    数：Y 指定字符串左上角的纵坐标，范围：0~63
 * 参    数：String 指定要显示的字符串，范围：ASCII码可见字符组成的字符串
 * 参    数：FontSize 指定字体大小
 *           范围：Int_OLED_8X16		宽8像素，高16像素
 *                 Int_OLED_6X8		宽6像素，高8像素
 * 返 回 值：无
 * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
 */
void Int_OLED_ShowString(uint8_t X, uint8_t Y, char *String, uint8_t FontSize)
{
    uint8_t i;
    for (i = 0; String[i] != '\0'; i++) // 遍历字符串的每个字符
    {
        /*调用Int_OLED_ShowChar函数，依次显示每个字符*/
        Int_OLED_ShowChar(X + i * FontSize, Y, String[i], FontSize);
    }
}

/**
 * 函    数：Int_OLED显示数字（十进制，正整数）
 * 参    数：X 指定数字左上角的横坐标，范围：0~127
 * 参    数：Y 指定数字左上角的纵坐标，范围：0~63
 * 参    数：Number 指定要显示的数字，范围：0~4294967295
 * 参    数：Length 指定数字的长度，范围：0~10
 * 参    数：FontSize 指定字体大小
 *           范围：Int_OLED_8X16		宽8像素，高16像素
 *                 Int_OLED_6X8		宽6像素，高8像素
 * 返 回 值：无
 * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
 */
void Int_OLED_ShowNum(uint8_t X, uint8_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize)
{
    uint8_t i;
    for (i = 0; i < Length; i++) // 遍历数字的每一位
    {
        /*调用Int_OLED_ShowChar函数，依次显示每个数字*/
        /*Number / Int_OLED_Pow(10, Length - i - 1) % 10 可以十进制提取数字的每一位*/
        /*+ '0' 可将数字转换为字符格式*/
        Int_OLED_ShowChar(X + i * FontSize, Y, Number / Int_OLED_Pow(10, Length - i - 1) % 10 + '0', FontSize);
    }
}

/**
 * 函    数：Int_OLED显示有符号数字（十进制，整数）
 * 参    数：X 指定数字左上角的横坐标，范围：0~127
 * 参    数：Y 指定数字左上角的纵坐标，范围：0~63
 * 参    数：Number 指定要显示的数字，范围：-2147483648~2147483647
 * 参    数：Length 指定数字的长度，范围：0~10
 * 参    数：FontSize 指定字体大小
 *           范围：Int_OLED_8X16		宽8像素，高16像素
 *                 Int_OLED_6X8		宽6像素，高8像素
 * 返 回 值：无
 * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
 */
void Int_OLED_ShowSignedNum(uint8_t X, uint8_t Y, int32_t Number, uint8_t Length, uint8_t FontSize)
{
    uint8_t i;
    uint32_t Number1;

    if (Number >= 0) // 数字大于等于0
    {
        Int_OLED_ShowChar(X, Y, '+', FontSize); // 显示+号
        Number1 = Number;                       // Number1直接等于Number
    }
    else // 数字小于0
    {
        Int_OLED_ShowChar(X, Y, '-', FontSize); // 显示-号
        Number1 = -Number;                      // Number1等于Number取负
    }

    for (i = 0; i < Length; i++) // 遍历数字的每一位
    {
        /*调用Int_OLED_ShowChar函数，依次显示每个数字*/
        /*Number1 / Int_OLED_Pow(10, Length - i - 1) % 10 可以十进制提取数字的每一位*/
        /*+ '0' 可将数字转换为字符格式*/
        Int_OLED_ShowChar(X + (i + 1) * FontSize, Y, Number1 / Int_OLED_Pow(10, Length - i - 1) % 10 + '0', FontSize);
    }
}

/**
 * 函    数：Int_OLED显示十六进制数字（十六进制，正整数）
 * 参    数：X 指定数字左上角的横坐标，范围：0~127
 * 参    数：Y 指定数字左上角的纵坐标，范围：0~63
 * 参    数：Number 指定要显示的数字，范围：0x00000000~0xFFFFFFFF
 * 参    数：Length 指定数字的长度，范围：0~8
 * 参    数：FontSize 指定字体大小
 *           范围：Int_OLED_8X16		宽8像素，高16像素
 *                 Int_OLED_6X8		宽6像素，高8像素
 * 返 回 值：无
 * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
 */
void Int_OLED_ShowHexNum(uint8_t X, uint8_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize)
{
    uint8_t i, SingleNumber;
    for (i = 0; i < Length; i++) // 遍历数字的每一位
    {
        /*以十六进制提取数字的每一位*/
        SingleNumber = Number / Int_OLED_Pow(16, Length - i - 1) % 16;

        if (SingleNumber < 10) // 单个数字小于10
        {
            /*调用Int_OLED_ShowChar函数，显示此数字*/
            /*+ '0' 可将数字转换为字符格式*/
            Int_OLED_ShowChar(X + i * FontSize, Y, SingleNumber + '0', FontSize);
        }
        else // 单个数字大于10
        {
            /*调用Int_OLED_ShowChar函数，显示此数字*/
            /*+ 'A' 可将数字转换为从A开始的十六进制字符*/
            Int_OLED_ShowChar(X + i * FontSize, Y, SingleNumber - 10 + 'A', FontSize);
        }
    }
}

/**
 * 函    数：Int_OLED显示二进制数字（二进制，正整数）
 * 参    数：X 指定数字左上角的横坐标，范围：0~127
 * 参    数：Y 指定数字左上角的纵坐标，范围：0~63
 * 参    数：Number 指定要显示的数字，范围：0x00000000~0xFFFFFFFF
 * 参    数：Length 指定数字的长度，范围：0~16
 * 参    数：FontSize 指定字体大小
 *           范围：Int_OLED_8X16		宽8像素，高16像素
 *                 Int_OLED_6X8		宽6像素，高8像素
 * 返 回 值：无
 * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
 */
void Int_OLED_ShowBinNum(uint8_t X, uint8_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize)
{
    uint8_t i;
    for (i = 0; i < Length; i++) // 遍历数字的每一位
    {
        /*调用Int_OLED_ShowChar函数，依次显示每个数字*/
        /*Number / Int_OLED_Pow(2, Length - i - 1) % 2 可以二进制提取数字的每一位*/
        /*+ '0' 可将数字转换为字符格式*/
        Int_OLED_ShowChar(X + i * FontSize, Y, Number / Int_OLED_Pow(2, Length - i - 1) % 2 + '0', FontSize);
    }
}

/**
 * 函    数：Int_OLED显示浮点数字（十进制，小数）
 * 参    数：X 指定数字左上角的横坐标，范围：0~127
 * 参    数：Y 指定数字左上角的纵坐标，范围：0~63
 * 参    数：Number 指定要显示的数字，范围：-4294967295.0~4294967295.0
 * 参    数：IntLength 指定数字的整数位长度，范围：0~10
 * 参    数：FraLength 指定数字的小数位长度，范围：0~9，过长的小数会有精度丢失
 * 参    数：FontSize 指定字体大小
 *           范围：Int_OLED_8X16		宽8像素，高16像素
 *                 Int_OLED_6X8		宽6像素，高8像素
 * 返 回 值：无
 * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
 */
void Int_OLED_ShowFloatNum(uint8_t X, uint8_t Y, double Number, uint8_t IntLength, uint8_t FraLength, uint8_t FontSize)
{
    uint32_t Temp;

    if (Number >= 0) // 数字大于等于0
    {
        /*显示整数部分*/
        Int_OLED_ShowNum(X, Y, Number, IntLength, FontSize);

        /*显示小数点*/
        Int_OLED_ShowChar(X + IntLength * FontSize, Y, '.', FontSize);

        /*将Number的整数部分减掉，防止之后将小数部分乘到整数时因数过大造成错误*/
        Number -= (uint32_t)Number;

        /*将小数部分乘到整数部分，并显示*/
        Temp = Int_OLED_Pow(10, FraLength);
        Int_OLED_ShowNum(X + (IntLength + 1) * FontSize, Y, ((uint32_t)(Number * Temp)) % Temp, FraLength, FontSize);
    }
    else // 数字小于0
    {
        Int_OLED_ShowChar(X, Y, '-', FontSize); // 显示-号
        Number = -Number;                       // Number取负
        /*显示整数部分*/
        Int_OLED_ShowNum(X + FontSize, Y, Number, IntLength, FontSize);

        /*显示小数点*/
        Int_OLED_ShowChar(X + (IntLength + 1) * FontSize, Y, '.', FontSize);

        /*将Number的整数部分减掉，防止之后将小数部分乘到整数时因数过大造成错误*/
        Number -= (uint32_t)Number;

        /*将小数部分乘到整数部分，并显示*/
        Temp = Int_OLED_Pow(10, FraLength);
        Int_OLED_ShowNum(X + (IntLength + 2) * FontSize, Y, ((uint32_t)(Number * Temp)) % Temp, FraLength, FontSize);
    }
}

/**
 * 函    数：Int_OLED显示图像
 * 参    数：X 指定图像左上角的横坐标，范围：0~127
 * 参    数：Y 指定图像左上角的纵坐标，范围：0~63
 * 参    数：Width 指定图像的宽度，范围：0~128
 * 参    数：Height 指定图像的高度，范围：0~64
 * 参    数：Image 指定要显示的图像
 * 返 回 值：无
 * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
 */
void Int_OLED_ShowImage(uint8_t X, uint8_t Y, uint8_t Width, uint8_t Height, const uint8_t *Image)
{
    uint8_t i, j;

    /*参数检查，保证指定图像不会超出屏幕范围*/
    if (X > 127)
    {
        return;
    }
    if (Y > 63)
    {
        return;
    }

    /*将图像所在区域清空*/
    Int_OLED_ClearArea(X, Y, Width, Height);

    /*遍历指定图像涉及的相关页*/
    /*(Height - 1) / 8 + 1的目的是Height / 8并向上取整*/
    for (j = 0; j < (Height - 1) / 8 + 1; j++)
    {
        /*遍历指定图像涉及的相关列*/
        for (i = 0; i < Width; i++)
        {
            /*超出边界，则跳过显示*/
            if (X + i > 127)
            {
                break;
            }
            if (Y / 8 + j > 7)
            {
                return;
            }

            /*显示图像在当前页的内容*/
            Int_OLED_DisplayBuf[Y / 8 + j][X + i] |= Image[j * Width + i] << (Y % 8);

            /*超出边界，则跳过显示*/
            /*使用continue的目的是，下一页超出边界时，上一页的后续内容还需要继续显示*/
            if (Y / 8 + j + 1 > 7)
            {
                continue;
            }

            /*显示图像在下一页的内容*/
            Int_OLED_DisplayBuf[Y / 8 + j + 1][X + i] |= Image[j * Width + i] >> (8 - Y % 8);
        }
    }
}

/**
 * 函    数：Int_OLED使用printf函数打印格式化字符串
 * 参    数：X 指定格式化字符串左上角的横坐标，范围：0~127
 * 参    数：Y 指定格式化字符串左上角的纵坐标，范围：0~63
 * 参    数：FontSize 指定字体大小
 *           范围：Int_OLED_8X16		宽8像素，高16像素
 *                 Int_OLED_6X8		宽6像素，高8像素
 * 参    数：format 指定要显示的格式化字符串，范围：ASCII码可见字符组成的字符串
 * 参    数：... 格式化字符串参数列表
 * 返 回 值：无
 * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
 */
void Int_OLED_Printf(uint8_t X, uint8_t Y, uint8_t FontSize, char *format, ...)
{
    char String[30];                             // 定义字符数组
    va_list arg;                                 // 定义可变参数列表数据类型的变量arg
    va_start(arg, format);                       // 从format开始，接收参数列表到arg变量
    vsprintf(String, format, arg);               // 使用vsprintf打印格式化字符串和参数列表到字符数组中
    va_end(arg);                                 // 结束变量arg
    Int_OLED_ShowString(X, Y, String, FontSize); // Int_OLED显示字符数组（字符串）
}

/**
 * 函    数：Int_OLED在指定位置画一个点
 * 参    数：X 指定点的横坐标，范围：0~127
 * 参    数：Y 指定点的纵坐标，范围：0~63
 * 返 回 值：无
 * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
 */
void Int_OLED_DrawPoint(uint8_t X, uint8_t Y)
{
    /*参数检查，保证指定位置不会超出屏幕范围*/
    if (X > 127)
    {
        return;
    }
    if (Y > 63)
    {
        return;
    }

    /*将显存数组指定位置的一个Bit数据置1*/
    Int_OLED_DisplayBuf[Y / 8][X] |= 0x01 << (Y % 8);
}

/**
 * 函    数：Int_OLED获取指定位置点的值
 * 参    数：X 指定点的横坐标，范围：0~127
 * 参    数：Y 指定点的纵坐标，范围：0~63
 * 返 回 值：指定位置点是否处于点亮状态，1：点亮，0：熄灭
 */
uint8_t Int_OLED_GetPoint(uint8_t X, uint8_t Y)
{
    /*参数检查，保证指定位置不会超出屏幕范围*/
    if (X > 127)
    {
        return 0;
    }
    if (Y > 63)
    {
        return 0;
    }

    /*判断指定位置的数据*/
    if (Int_OLED_DisplayBuf[Y / 8][X] & 0x01 << (Y % 8))
    {
        return 1; // 为1，返回1
    }

    return 0; // 否则，返回0
}

/**
 * 函    数：Int_OLED画线
 * 参    数：X0 指定一个端点的横坐标，范围：0~127
 * 参    数：Y0 指定一个端点的纵坐标，范围：0~63
 * 参    数：X1 指定另一个端点的横坐标，范围：0~127
 * 参    数：Y1 指定另一个端点的纵坐标，范围：0~63
 * 返 回 值：无
 * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
 */
void Int_OLED_DrawLine(uint8_t X0, uint8_t Y0, uint8_t X1, uint8_t Y1)
{
    int16_t x, y, dx, dy, d, incrE, incrNE, temp;
    int16_t x0 = X0, y0 = Y0, x1 = X1, y1 = Y1;
    uint8_t yflag = 0, xyflag = 0;

    if (y0 == y1) // 横线单独处理
    {
        /*0号点X坐标大于1号点X坐标，则交换两点X坐标*/
        if (x0 > x1)
        {
            temp = x0;
            x0 = x1;
            x1 = temp;
        }

        /*遍历X坐标*/
        for (x = x0; x <= x1; x++)
        {
            Int_OLED_DrawPoint(x, y0); // 依次画点
        }
    }
    else if (x0 == x1) // 竖线单独处理
    {
        /*0号点Y坐标大于1号点Y坐标，则交换两点Y坐标*/
        if (y0 > y1)
        {
            temp = y0;
            y0 = y1;
            y1 = temp;
        }

        /*遍历Y坐标*/
        for (y = y0; y <= y1; y++)
        {
            Int_OLED_DrawPoint(x0, y); // 依次画点
        }
    }
    else // 斜线
    {
        if (x0 > x1) // 0号点X坐标大于1号点X坐标
        {
            /*交换两点坐标*/
            /*交换后不影响画线，但是画线方向由第一、二、三、四象限变为第一、四象限*/
            temp = x0;
            x0 = x1;
            x1 = temp;
            temp = y0;
            y0 = y1;
            y1 = temp;
        }

        if (y0 > y1) // 0号点Y坐标大于1号点Y坐标
        {
            /*将Y坐标取负*/
            /*取负后影响画线，但是画线方向由第一、四象限变为第一象限*/
            y0 = -y0;
            y1 = -y1;

            /*置标志位yflag，记住当前变换，在后续实际画线时，再将坐标换回来*/
            yflag = 1;
        }

        if (y1 - y0 > x1 - x0) // 画线斜率大于1
        {
            /*将X坐标与Y坐标互换*/
            /*互换后影响画线，但是画线方向由第一象限0~90度范围变为第一象限0~45度范围*/
            temp = x0;
            x0 = y0;
            y0 = temp;
            temp = x1;
            x1 = y1;
            y1 = temp;

            /*置标志位xyflag，记住当前变换，在后续实际画线时，再将坐标换回来*/
            xyflag = 1;
        }

        /*以下为Bresenham算法画直线*/
        /*算法要求，画线方向必须为第一象限0~45度范围*/
        dx = x1 - x0;
        dy = y1 - y0;
        incrE = 2 * dy;
        incrNE = 2 * (dy - dx);
        d = 2 * dy - dx;
        x = x0;
        y = y0;

        /*画起始点，同时判断标志位，将坐标换回来*/
        if (yflag && xyflag)
        {
            Int_OLED_DrawPoint(y, -x);
        }
        else if (yflag)
        {
            Int_OLED_DrawPoint(x, -y);
        }
        else if (xyflag)
        {
            Int_OLED_DrawPoint(y, x);
        }
        else
        {
            Int_OLED_DrawPoint(x, y);
        }

        while (x < x1) // 遍历X轴的每个点
        {
            x++;
            if (d < 0) // 下一个点在当前点东方
            {
                d += incrE;
            }
            else // 下一个点在当前点东北方
            {
                y++;
                d += incrNE;
            }

            /*画每一个点，同时判断标志位，将坐标换回来*/
            if (yflag && xyflag)
            {
                Int_OLED_DrawPoint(y, -x);
            }
            else if (yflag)
            {
                Int_OLED_DrawPoint(x, -y);
            }
            else if (xyflag)
            {
                Int_OLED_DrawPoint(y, x);
            }
            else
            {
                Int_OLED_DrawPoint(x, y);
            }
        }
    }
}

/**
 * 函    数：Int_OLED矩形
 * 参    数：X 指定矩形左上角的横坐标，范围：0~127
 * 参    数：Y 指定矩形左上角的纵坐标，范围：0~63
 * 参    数：Width 指定矩形的宽度，范围：0~128
 * 参    数：Height 指定矩形的高度，范围：0~64
 * 参    数：IsFilled 指定矩形是否填充
 *           范围：Int_OLED_UNFILLED		不填充
 *                 Int_OLED_FILLED			填充
 * 返 回 值：无
 * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
 */
void Int_OLED_DrawRectangle(uint8_t X, uint8_t Y, uint8_t Width, uint8_t Height, uint8_t IsFilled)
{
    uint8_t i, j;
    if (!IsFilled) // 指定矩形不填充
    {
        /*遍历上下X坐标，画矩形上下两条线*/
        for (i = X; i < X + Width; i++)
        {
            Int_OLED_DrawPoint(i, Y);
            Int_OLED_DrawPoint(i, Y + Height - 1);
        }
        /*遍历左右Y坐标，画矩形左右两条线*/
        for (i = Y; i < Y + Height; i++)
        {
            Int_OLED_DrawPoint(X, i);
            Int_OLED_DrawPoint(X + Width - 1, i);
        }
    }
    else // 指定矩形填充
    {
        /*遍历X坐标*/
        for (i = X; i < X + Width; i++)
        {
            /*遍历Y坐标*/
            for (j = Y; j < Y + Height; j++)
            {
                /*在指定区域画点，填充满矩形*/
                Int_OLED_DrawPoint(i, j);
            }
        }
    }
}

/**
 * 函    数：Int_OLED三角形
 * 参    数：X0 指定第一个端点的横坐标，范围：0~127
 * 参    数：Y0 指定第一个端点的纵坐标，范围：0~63
 * 参    数：X1 指定第二个端点的横坐标，范围：0~127
 * 参    数：Y1 指定第二个端点的纵坐标，范围：0~63
 * 参    数：X2 指定第三个端点的横坐标，范围：0~127
 * 参    数：Y2 指定第三个端点的纵坐标，范围：0~63
 * 参    数：IsFilled 指定三角形是否填充
 *           范围：Int_OLED_UNFILLED		不填充
 *                 Int_OLED_FILLED			填充
 * 返 回 值：无
 * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
 */
void Int_OLED_DrawTriangle(uint8_t X0, uint8_t Y0, uint8_t X1, uint8_t Y1, uint8_t X2, uint8_t Y2, uint8_t IsFilled)
{
    uint8_t minx = X0, miny = Y0, maxx = X0, maxy = Y0;
    uint8_t i, j;
    int16_t vx[] = {X0, X1, X2};
    int16_t vy[] = {Y0, Y1, Y2};

    if (!IsFilled) // 指定三角形不填充
    {
        /*调用画线函数，将三个点用直线连接*/
        Int_OLED_DrawLine(X0, Y0, X1, Y1);
        Int_OLED_DrawLine(X0, Y0, X2, Y2);
        Int_OLED_DrawLine(X1, Y1, X2, Y2);
    }
    else // 指定三角形填充
    {
        /*找到三个点最小的X、Y坐标*/
        if (X1 < minx)
        {
            minx = X1;
        }
        if (X2 < minx)
        {
            minx = X2;
        }
        if (Y1 < miny)
        {
            miny = Y1;
        }
        if (Y2 < miny)
        {
            miny = Y2;
        }

        /*找到三个点最大的X、Y坐标*/
        if (X1 > maxx)
        {
            maxx = X1;
        }
        if (X2 > maxx)
        {
            maxx = X2;
        }
        if (Y1 > maxy)
        {
            maxy = Y1;
        }
        if (Y2 > maxy)
        {
            maxy = Y2;
        }

        /*最小最大坐标之间的矩形为可能需要填充的区域*/
        /*遍历此区域中所有的点*/
        /*遍历X坐标*/
        for (i = minx; i <= maxx; i++)
        {
            /*遍历Y坐标*/
            for (j = miny; j <= maxy; j++)
            {
                /*调用Int_OLED_pnpoly，判断指定点是否在指定三角形之中*/
                /*如果在，则画点，如果不在，则不做处理*/
                if (Int_OLED_pnpoly(3, vx, vy, i, j))
                {
                    Int_OLED_DrawPoint(i, j);
                }
            }
        }
    }
}

/**
 * 函    数：Int_OLED画圆
 * 参    数：X 指定圆的圆心横坐标，范围：0~127
 * 参    数：Y 指定圆的圆心纵坐标，范围：0~63
 * 参    数：Radius 指定圆的半径，范围：0~255
 * 参    数：IsFilled 指定圆是否填充
 *           范围：Int_OLED_UNFILLED		不填充
 *                 Int_OLED_FILLED			填充
 * 返 回 值：无
 * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
 */
void Int_OLED_DrawCircle(uint8_t X, uint8_t Y, uint8_t Radius, uint8_t IsFilled)
{
    int16_t x, y, d, j;

    d = 1 - Radius;
    x = 0;
    y = Radius;

    /*画每个八分之一圆弧的起始点*/
    Int_OLED_DrawPoint(X + x, Y + y);
    Int_OLED_DrawPoint(X - x, Y - y);
    Int_OLED_DrawPoint(X + y, Y + x);
    Int_OLED_DrawPoint(X - y, Y - x);

    if (IsFilled) // 指定圆填充
    {
        /*遍历起始点Y坐标*/
        for (j = -y; j < y; j++)
        {
            /*在指定区域画点，填充部分圆*/
            Int_OLED_DrawPoint(X, Y + j);
        }
    }

    while (x < y) // 遍历X轴的每个点
    {
        x++;
        if (d < 0) // 下一个点在当前点东方
        {
            d += 2 * x + 1;
        }
        else // 下一个点在当前点东南方
        {
            y--;
            d += 2 * (x - y) + 1;
        }

        /*画每个八分之一圆弧的点*/
        Int_OLED_DrawPoint(X + x, Y + y);
        Int_OLED_DrawPoint(X + y, Y + x);
        Int_OLED_DrawPoint(X - x, Y - y);
        Int_OLED_DrawPoint(X - y, Y - x);
        Int_OLED_DrawPoint(X + x, Y - y);
        Int_OLED_DrawPoint(X + y, Y - x);
        Int_OLED_DrawPoint(X - x, Y + y);
        Int_OLED_DrawPoint(X - y, Y + x);

        if (IsFilled) // 指定圆填充
        {
            /*遍历中间部分*/
            for (j = -y; j < y; j++)
            {
                /*在指定区域画点，填充部分圆*/
                Int_OLED_DrawPoint(X + x, Y + j);
                Int_OLED_DrawPoint(X - x, Y + j);
            }

            /*遍历两侧部分*/
            for (j = -x; j < x; j++)
            {
                /*在指定区域画点，填充部分圆*/
                Int_OLED_DrawPoint(X - y, Y + j);
                Int_OLED_DrawPoint(X + y, Y + j);
            }
        }
    }
}

/**
 * 函    数：Int_OLED画椭圆
 * 参    数：X 指定椭圆的圆心横坐标，范围：0~127
 * 参    数：Y 指定椭圆的圆心纵坐标，范围：0~63
 * 参    数：A 指定椭圆的横向半轴长度，范围：0~255
 * 参    数：B 指定椭圆的纵向半轴长度，范围：0~255
 * 参    数：IsFilled 指定椭圆是否填充
 *           范围：Int_OLED_UNFILLED		不填充
 *                 Int_OLED_FILLED			填充
 * 返 回 值：无
 * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
 */
void Int_OLED_DrawEllipse(uint8_t X, uint8_t Y, uint8_t A, uint8_t B, uint8_t IsFilled)
{
    int16_t x, y, j;
    int16_t a = A, b = B;
    float d1, d2;

    x = 0;
    y = b;
    d1 = b * b + a * a * (-b + 0.5);

    if (IsFilled) // 指定椭圆填充
    {
        /*遍历起始点Y坐标*/
        for (j = -y; j < y; j++)
        {
            /*在指定区域画点，填充部分椭圆*/
            Int_OLED_DrawPoint(X, Y + j);
            Int_OLED_DrawPoint(X, Y + j);
        }
    }

    /*画椭圆弧的起始点*/
    Int_OLED_DrawPoint(X + x, Y + y);
    Int_OLED_DrawPoint(X - x, Y - y);
    Int_OLED_DrawPoint(X - x, Y + y);
    Int_OLED_DrawPoint(X + x, Y - y);

    /*画椭圆中间部分*/
    while (b * b * (x + 1) < a * a * (y - 0.5))
    {
        if (d1 <= 0) // 下一个点在当前点东方
        {
            d1 += b * b * (2 * x + 3);
        }
        else // 下一个点在当前点东南方
        {
            d1 += b * b * (2 * x + 3) + a * a * (-2 * y + 2);
            y--;
        }
        x++;

        if (IsFilled) // 指定椭圆填充
        {
            /*遍历中间部分*/
            for (j = -y; j < y; j++)
            {
                /*在指定区域画点，填充部分椭圆*/
                Int_OLED_DrawPoint(X + x, Y + j);
                Int_OLED_DrawPoint(X - x, Y + j);
            }
        }

        /*画椭圆中间部分圆弧*/
        Int_OLED_DrawPoint(X + x, Y + y);
        Int_OLED_DrawPoint(X - x, Y - y);
        Int_OLED_DrawPoint(X - x, Y + y);
        Int_OLED_DrawPoint(X + x, Y - y);
    }

    /*画椭圆两侧部分*/
    d2 = b * b * (x + 0.5) * (x + 0.5) + a * a * (y - 1) * (y - 1) - a * a * b * b;

    while (y > 0)
    {
        if (d2 <= 0) // 下一个点在当前点东方
        {
            d2 += b * b * (2 * x + 2) + a * a * (-2 * y + 3);
            x++;
        }
        else // 下一个点在当前点东南方
        {
            d2 += a * a * (-2 * y + 3);
        }
        y--;

        if (IsFilled) // 指定椭圆填充
        {
            /*遍历两侧部分*/
            for (j = -y; j < y; j++)
            {
                /*在指定区域画点，填充部分椭圆*/
                Int_OLED_DrawPoint(X + x, Y + j);
                Int_OLED_DrawPoint(X - x, Y + j);
            }
        }

        /*画椭圆两侧部分圆弧*/
        Int_OLED_DrawPoint(X + x, Y + y);
        Int_OLED_DrawPoint(X - x, Y - y);
        Int_OLED_DrawPoint(X - x, Y + y);
        Int_OLED_DrawPoint(X + x, Y - y);
    }
}

/**
 * 函    数：Int_OLED画圆弧
 * 参    数：X 指定圆弧的圆心横坐标，范围：0~127
 * 参    数：Y 指定圆弧的圆心纵坐标，范围：0~63
 * 参    数：Radius 指定圆弧的半径，范围：0~255
 * 参    数：StartAngle 指定圆弧的起始角度，范围：-180~180
 *           水平向右为0度，水平向左为180度或-180度，下方为正数，上方为负数，顺时针旋转
 * 参    数：EndAngle 指定圆弧的终止角度，范围：-180~180
 *           水平向右为0度，水平向左为180度或-180度，下方为正数，上方为负数，顺时针旋转
 * 参    数：IsFilled 指定圆弧是否填充，填充后为扇形
 *           范围：Int_OLED_UNFILLED		不填充
 *                 Int_OLED_FILLED			填充
 * 返 回 值：无
 * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
 */
void Int_OLED_DrawArc(uint8_t X, uint8_t Y, uint8_t Radius, int16_t StartAngle, int16_t EndAngle, uint8_t IsFilled)
{
    int16_t x, y, d, j;

    /*此函数借用Bresenham算法画圆的方法*/

    d = 1 - Radius;
    x = 0;
    y = Radius;

    /*在画圆的每个点时，判断指定点是否在指定角度内，在，则画点，不在，则不做处理*/
    if (Int_OLED_IsInAngle(x, y, StartAngle, EndAngle))
    {
        Int_OLED_DrawPoint(X + x, Y + y);
    }
    if (Int_OLED_IsInAngle(-x, -y, StartAngle, EndAngle))
    {
        Int_OLED_DrawPoint(X - x, Y - y);
    }
    if (Int_OLED_IsInAngle(y, x, StartAngle, EndAngle))
    {
        Int_OLED_DrawPoint(X + y, Y + x);
    }
    if (Int_OLED_IsInAngle(-y, -x, StartAngle, EndAngle))
    {
        Int_OLED_DrawPoint(X - y, Y - x);
    }

    if (IsFilled) // 指定圆弧填充
    {
        /*遍历起始点Y坐标*/
        for (j = -y; j < y; j++)
        {
            /*在填充圆的每个点时，判断指定点是否在指定角度内，在，则画点，不在，则不做处理*/
            if (Int_OLED_IsInAngle(0, j, StartAngle, EndAngle))
            {
                Int_OLED_DrawPoint(X, Y + j);
            }
        }
    }

    while (x < y) // 遍历X轴的每个点
    {
        x++;
        if (d < 0) // 下一个点在当前点东方
        {
            d += 2 * x + 1;
        }
        else // 下一个点在当前点东南方
        {
            y--;
            d += 2 * (x - y) + 1;
        }

        /*在画圆的每个点时，判断指定点是否在指定角度内，在，则画点，不在，则不做处理*/
        if (Int_OLED_IsInAngle(x, y, StartAngle, EndAngle))
        {
            Int_OLED_DrawPoint(X + x, Y + y);
        }
        if (Int_OLED_IsInAngle(y, x, StartAngle, EndAngle))
        {
            Int_OLED_DrawPoint(X + y, Y + x);
        }
        if (Int_OLED_IsInAngle(-x, -y, StartAngle, EndAngle))
        {
            Int_OLED_DrawPoint(X - x, Y - y);
        }
        if (Int_OLED_IsInAngle(-y, -x, StartAngle, EndAngle))
        {
            Int_OLED_DrawPoint(X - y, Y - x);
        }
        if (Int_OLED_IsInAngle(x, -y, StartAngle, EndAngle))
        {
            Int_OLED_DrawPoint(X + x, Y - y);
        }
        if (Int_OLED_IsInAngle(y, -x, StartAngle, EndAngle))
        {
            Int_OLED_DrawPoint(X + y, Y - x);
        }
        if (Int_OLED_IsInAngle(-x, y, StartAngle, EndAngle))
        {
            Int_OLED_DrawPoint(X - x, Y + y);
        }
        if (Int_OLED_IsInAngle(-y, x, StartAngle, EndAngle))
        {
            Int_OLED_DrawPoint(X - y, Y + x);
        }

        if (IsFilled) // 指定圆弧填充
        {
            /*遍历中间部分*/
            for (j = -y; j < y; j++)
            {
                /*在填充圆的每个点时，判断指定点是否在指定角度内，在，则画点，不在，则不做处理*/
                if (Int_OLED_IsInAngle(x, j, StartAngle, EndAngle))
                {
                    Int_OLED_DrawPoint(X + x, Y + j);
                }
                if (Int_OLED_IsInAngle(-x, j, StartAngle, EndAngle))
                {
                    Int_OLED_DrawPoint(X - x, Y + j);
                }
            }

            /*遍历两侧部分*/
            for (j = -x; j < x; j++)
            {
                /*在填充圆的每个点时，判断指定点是否在指定角度内，在，则画点，不在，则不做处理*/
                if (Int_OLED_IsInAngle(-y, j, StartAngle, EndAngle))
                {
                    Int_OLED_DrawPoint(X - y, Y + j);
                }
                if (Int_OLED_IsInAngle(y, j, StartAngle, EndAngle))
                {
                    Int_OLED_DrawPoint(X + y, Y + j);
                }
            }
        }
    }
}
