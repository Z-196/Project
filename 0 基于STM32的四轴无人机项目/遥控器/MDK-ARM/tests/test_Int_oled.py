"""
Unit tests for Int_oled.h / Int_oled.c OLED display driver.
Tests the pure logic functions ported to Python.

This file tests the exact same logic as the C code:
  - OLED_SetPixel
  - OLED_SetByte / OLED_SetByte_Fine
  - OLED_SetBits / OLED_SetBits_Fine
  - OLED_SetBlock
  - OLED_NewFrame
  - OLED_DrawLine (Bresenham)
  - OLED_DrawRectangle / OLED_DrawFilledRectangle
  - OLED_DrawTriangle
  - OLED_DrawCircle (Bresenham)
  - OLED_DrawFilledCircle
  - OLED_DrawEllipse
  - OLED_DrawImage
  - OLED_PrintASCIIChar / OLED_PrintASCIIString
  - OLED_PrintString
  - _OLED_GetUTF8Len
"""

import math
import ctypes
import sys

# ===================== OLED Constants =====================
OLED_PAGE = 8
OLED_ROW = 8 * OLED_PAGE  # 64
OLED_COLUMN = 128

# ===================== OLED_ColorMode =====================
OLED_COLOR_NORMAL = 0
OLED_COLOR_REVERSED = 1

# ===================== GRAM (display RAM) =====================
OLED_GRAM = [[0] * OLED_COLUMN for _ in range(OLED_PAGE)]


# ===================== Helper Functions =====================

def clear_gram():
    """Reset GRAM to all zeros."""
    global OLED_GRAM
    OLED_GRAM = [[0] * OLED_COLUMN for _ in range(OLED_PAGE)]


def count_set_pixels():
    """Count set bits (pixels) in GRAM."""
    count = 0
    for page in range(OLED_PAGE):
        for col in range(OLED_COLUMN):
            byte = OLED_GRAM[page][col]
            while byte:
                count += byte & 1
                byte >>= 1
    return count


def is_pixel_set(x, y):
    """Check if a pixel is set in GRAM (NORMAL mode)."""
    if x >= OLED_COLUMN or y >= OLED_ROW:
        return False
    return (OLED_GRAM[y // 8][x] >> (y % 8)) & 0x01


# ===================== Ported C Functions =====================

def OLED_NewFrame():
    """Clear GRAM - prepare new frame."""
    global OLED_GRAM
    OLED_GRAM = [[0] * OLED_COLUMN for _ in range(OLED_PAGE)]


def OLED_SetPixel(x, y, color):
    """Set a single pixel in GRAM."""
    if x >= OLED_COLUMN or y >= OLED_ROW:
        return
    if not color:  # OLED_COLOR_NORMAL
        OLED_GRAM[y // 8][x] |= 1 << (y % 8)
    else:  # OLED_COLOR_REVERSED
        OLED_GRAM[y // 8][x] &= ~(1 << (y % 8))


def OLED_SetByte_Fine(page, column, data, start, end, color):
    """Set specific bits of a byte in GRAM."""
    if page >= OLED_PAGE or column >= OLED_COLUMN:
        return
    if color:
        data = (~data) & 0xFF

    temp = data | (0xFF << (end + 1)) | (0xFF >> (8 - start))
    OLED_GRAM[page][column] &= temp
    temp = data & ~(0xFF << (end + 1)) & ~(0xFF >> (8 - start))
    OLED_GRAM[page][column] |= temp


def OLED_SetByte(page, column, data, color):
    """Set a full byte in GRAM."""
    if page >= OLED_PAGE or column >= OLED_COLUMN:
        return
    if color:
        data = (~data) & 0xFF
    OLED_GRAM[page][column] = data


def OLED_SetBits_Fine(x, y, data, length, color):
    """Set bits potentially crossing page boundary."""
    page = y // 8
    bit = y % 8
    if bit + length > 8:
        OLED_SetByte_Fine(page, x, (data << bit) & 0xFF, bit, 7, color)
        OLED_SetByte_Fine(page + 1, x, (data >> (8 - bit)) & 0xFF, 0, length + bit - 1 - 8, color)
    else:
        OLED_SetByte_Fine(page, x, (data << bit) & 0xFF, bit, bit + length - 1, color)


def OLED_SetBits(x, y, data, color):
    """Set 8 bits, potentially crossing page boundary."""
    page = y // 8
    bit = y % 8
    OLED_SetByte_Fine(page, x, (data << bit) & 0xFF, bit, 7, color)
    if bit:
        OLED_SetByte_Fine(page + 1, x, (data >> (8 - bit)) & 0xFF, 0, bit - 1, color)


def OLED_SetBlock(x, y, data, w, h, color):
    """Set a block of pixels from column-major data."""
    fullRow = h // 8
    partBit = h % 8
    for i in range(w):
        for j in range(fullRow):
            OLED_SetBits(x + i, y + j * 8, data[i + j * w], color)
    if partBit:
        fullNum = w * fullRow
        for i in range(w):
            OLED_SetBits_Fine(x + i, y + (fullRow * 8), data[fullNum + i], partBit, color)


def OLED_DrawLine(x1, y1, x2, y2, color):
    """Draw a line using Bresenham algorithm."""
    temp = 0
    if x1 == x2:
        if y1 > y2:
            temp = y1
            y1 = y2
            y2 = temp
        for y in range(y1, y2 + 1):
            OLED_SetPixel(x1, y, color)
    elif y1 == y2:
        if x1 > x2:
            temp = x1
            x1 = x2
            x2 = temp
        for x in range(x1, x2 + 1):
            OLED_SetPixel(x, y1, color)
    else:
        dx = x2 - x1
        dy = y2 - y1
        ux = 1 if dx > 0 else -1
        uy = 1 if dy > 0 else -1
        x, y, eps = x1, y1, 0
        dx = abs(dx)
        dy = abs(dy)
        if dx > dy:
            x = x1
            while x != x2:
                OLED_SetPixel(x, y, color)
                eps += dy
                if (eps << 1) >= dx:
                    y += uy
                    eps -= dx
                x += ux
        else:
            y = y1
            while y != y2:
                OLED_SetPixel(x, y, color)
                eps += dx
                if (eps << 1) >= dy:
                    x += ux
                    eps -= dy
                y += uy


def OLED_DrawRectangle(x, y, w, h, color):
    """Draw a rectangle outline."""
    OLED_DrawLine(x, y, x + w, y, color)
    OLED_DrawLine(x, y + h, x + w, y + h, color)
    OLED_DrawLine(x, y, x, y + h, color)
    OLED_DrawLine(x + w, y, x + w, y + h, color)


def OLED_DrawFilledRectangle(x, y, w, h, color):
    """Draw a filled rectangle."""
    for i in range(h):
        OLED_DrawLine(x, y + i, x + w, y + i, color)


def OLED_DrawTriangle(x1, y1, x2, y2, x3, y3, color):
    """Draw a triangle outline."""
    OLED_DrawLine(x1, y1, x2, y2, color)
    OLED_DrawLine(x2, y2, x3, y3, color)
    OLED_DrawLine(x3, y3, x1, y1, color)


def OLED_DrawFilledTriangle(x1, y1, x2, y2, x3, y3, color):
    """Draw a filled triangle (matches C implementation behavior)."""
    # Sort y1, y2: a = min(y1,y2), b = max(y1,y2)
    if y1 > y2:
        a, b = y2, y1
    else:
        a, b = y1, y2
    y = a
    last = 0
    for y_val in range(a, b + 1):
        if y_val <= y3:
            # C code would divide by zero if y2==y1 or y3==y1
            if (y2 - y1) != 0 and (y3 - y1) != 0:
                OLED_DrawLine(
                    x1 + (y_val - y1) * (x2 - x1) // (y2 - y1), y_val,
                    x1 + (y_val - y1) * (x3 - x1) // (y3 - y1), y_val, color
                )
            y = y_val
        else:
            last = y_val - 1
            break
    for y_val in range(y + 1, b + 1):
        if (y3 - y2) != 0 and (y3 - last) != 0:
            OLED_DrawLine(
                x2 + (y_val - y2) * (x3 - x2) // (y3 - y2), y_val,
                x1 + (y_val - last) * (x3 - x1) // (y3 - last), y_val, color
            )


def OLED_DrawCircle(x, y, r, color):
    """Draw a circle outline using Bresenham algorithm."""
    a, b = 0, r
    di = 3 - (r << 1)
    while a <= b:
        OLED_SetPixel(x - b, y - a, color)
        OLED_SetPixel(x + b, y - a, color)
        OLED_SetPixel(x - a, y + b, color)
        OLED_SetPixel(x - b, y - a, color)
        OLED_SetPixel(x - a, y - b, color)
        OLED_SetPixel(x + b, y + a, color)
        OLED_SetPixel(x + a, y - b, color)
        OLED_SetPixel(x + a, y + b, color)
        OLED_SetPixel(x - b, y + a, color)
        a += 1
        if di < 0:
            di += 4 * a + 6
        else:
            di += 10 + 4 * (a - b)
            b -= 1
        OLED_SetPixel(x + a, y + b, color)


def OLED_DrawFilledCircle(x, y, r, color):
    """Draw a filled circle."""
    a, b = 0, r
    di = 3 - (r << 1)
    while a <= b:
        for i in range(x - b, x + b + 1):
            OLED_SetPixel(i, y + a, color)
            OLED_SetPixel(i, y - a, color)
        for i in range(x - a, x + a + 1):
            OLED_SetPixel(i, y + b, color)
            OLED_SetPixel(i, y - b, color)
        a += 1
        if di < 0:
            di += 4 * a + 6
        else:
            di += 10 + 4 * (a - b)
            b -= 1


def OLED_DrawEllipse(x, y, a, b, color):
    """Draw an ellipse outline."""
    xpos, ypos = 0, b
    a2, b2 = a * a, b * b
    d = b2 + a2 * (0.25 - b)
    while a2 * ypos > b2 * xpos:
        OLED_SetPixel(x + xpos, y + ypos, color)
        OLED_SetPixel(x - xpos, y + ypos, color)
        OLED_SetPixel(x + xpos, y - ypos, color)
        OLED_SetPixel(x - xpos, y - ypos, color)
        if d < 0:
            d = d + b2 * ((xpos << 1) + 3)
            xpos += 1
        else:
            d = d + b2 * ((xpos << 1) + 3) + a2 * (-(ypos << 1) + 2)
            xpos += 1
            ypos -= 1
    d = b2 * (xpos + 0.5) * (xpos + 0.5) + a2 * (ypos - 1) * (ypos - 1) - a2 * b2
    while ypos > 0:
        OLED_SetPixel(x + xpos, y + ypos, color)
        OLED_SetPixel(x - xpos, y + ypos, color)
        OLED_SetPixel(x + xpos, y - ypos, color)
        OLED_SetPixel(x - xpos, y - ypos, color)
        if d < 0:
            d = d + b2 * ((xpos << 1) + 2) + a2 * (-(ypos << 1) + 3)
            xpos += 1
            ypos -= 1
        else:
            d = d + a2 * (-(ypos << 1) + 3)
            ypos -= 1


def OLED_DrawImage(x, y, img_data, w, h, color):
    """Draw an image from data."""
    OLED_SetBlock(x, y, img_data, w, h, color)


def OLED_PrintASCIIChar(x, y, ch, font_h, font_w, font_chars, color):
    """Draw an ASCII character using font data."""
    char_offset = (ord(ch) - ord(' ')) * (((font_h + 7) // 8) * font_w)
    OLED_SetBlock(x, y, font_chars[char_offset:], font_w, font_h, color)


def OLED_PrintASCIIString(x, y, s, font_h, font_w, font_chars, color):
    """Draw an ASCII string."""
    x0 = x
    for ch in s:
        OLED_PrintASCIIChar(x0, y, ch, font_h, font_w, font_chars, color)
        x0 += font_w


def _OLED_GetUTF8Len(s):
    """Get UTF-8 character byte length.
    In the C code, 's' is a char* pointing to a UTF-8 byte sequence.
    In Python, we accept either a bytes object or a string.
    If string, we encode to UTF-8 bytes first.
    """
    if isinstance(s, str):
        s = s.encode('utf-8')
    if len(s) == 0:
        return 0
    first = s[0]
    if (first & 0x80) == 0x00:
        return 1
    elif (first & 0xE0) == 0xC0:
        return 2
    elif (first & 0xF0) == 0xE0:
        return 3
    elif (first & 0xF8) == 0xF0:
        return 4
    return 0


# ===================== Test Framework =====================

passed = 0
failed = 0


def run_test(name, test_func):
    global passed, failed
    print(f"  Running {name}...", end="")
    try:
        test_func()
        passed += 1
        print(" PASSED")
    except AssertionError as e:
        failed += 1
        print(f" FAILED: {e}")
    except Exception as e:
        failed += 1
        print(f" ERROR: {e}")


def assert_true(condition, msg="Expected TRUE"):
    if not condition:
        raise AssertionError(msg)


def assert_false(condition, msg="Expected FALSE"):
    if condition:
        raise AssertionError(msg)


def assert_equal(expected, actual, msg=None):
    if expected != actual:
        raise AssertionError(msg or f"Expected {expected}, got {actual}")


# ===================== Test: OLED_SetPixel =====================

def test_SetPixel_Normal_InsideBounds():
    clear_gram()
    OLED_SetPixel(10, 20, OLED_COLOR_NORMAL)
    assert_true(is_pixel_set(10, 20))
    assert_equal(1, count_set_pixels())


def test_SetPixel_Normal_OutOfBounds():
    clear_gram()
    OLED_SetPixel(200, 20, OLED_COLOR_NORMAL)
    OLED_SetPixel(10, 200, OLED_COLOR_NORMAL)
    OLED_SetPixel(200, 200, OLED_COLOR_NORMAL)
    assert_equal(0, count_set_pixels())


def test_SetPixel_Reversed_ClearsPixel():
    clear_gram()
    OLED_SetPixel(50, 10, OLED_COLOR_NORMAL)
    assert_true(is_pixel_set(50, 10))
    OLED_SetPixel(50, 10, OLED_COLOR_REVERSED)
    assert_false(is_pixel_set(50, 10))


def test_SetPixel_MultiplePixels():
    clear_gram()
    for x in range(100):
        OLED_SetPixel(x, 0, OLED_COLOR_NORMAL)
    assert_equal(100, count_set_pixels())


def test_SetPixel_BoundaryConditions():
    clear_gram()
    # Maximum valid coordinates
    OLED_SetPixel(127, 63, OLED_COLOR_NORMAL)
    assert_true(is_pixel_set(127, 63))
    # Just outside
    OLED_SetPixel(128, 63, OLED_COLOR_NORMAL)
    OLED_SetPixel(127, 64, OLED_COLOR_NORMAL)
    assert_equal(1, count_set_pixels())


# ===================== Test: OLED_NewFrame =====================

def test_NewFrame_ClearsGRAM():
    clear_gram()
    OLED_SetPixel(10, 10, OLED_COLOR_NORMAL)
    OLED_SetPixel(20, 20, OLED_COLOR_NORMAL)
    OLED_SetPixel(30, 30, OLED_COLOR_NORMAL)
    assert_equal(3, count_set_pixels())
    OLED_NewFrame()
    assert_equal(0, count_set_pixels())


# ===================== Test: OLED_SetByte =====================

def test_SetByte_Normal():
    clear_gram()
    OLED_SetByte(0, 0, 0xFF, OLED_COLOR_NORMAL)
    assert_equal(0xFF, OLED_GRAM[0][0])


def test_SetByte_Reversed():
    clear_gram()
    OLED_SetByte(0, 0, 0x0F, OLED_COLOR_REVERSED)
    assert_equal((~0x0F) & 0xFF, OLED_GRAM[0][0])


def test_SetByte_OutOfBounds():
    clear_gram()
    OLED_SetByte(10, 0, 0xFF, OLED_COLOR_NORMAL)
    OLED_SetByte(0, 130, 0xFF, OLED_COLOR_NORMAL)
    assert_equal(0, count_set_pixels())


# ===================== Test: OLED_SetByte_Fine =====================

def test_SetByte_Fine_Normal_SetMiddleBits():
    clear_gram()
    OLED_SetByte_Fine(0, 0, 0xFF, 2, 5, OLED_COLOR_NORMAL)
    assert_equal(0x3C, OLED_GRAM[0][0])  # bits 2,3,4,5 = 0011_1100


def test_SetByte_Fine_Reversed_ClearsBits():
    clear_gram()
    OLED_GRAM[0][0] = 0xFF
    OLED_SetByte_Fine(0, 0, 0xFF, 2, 5, OLED_COLOR_REVERSED)
    assert_equal(0xC3, OLED_GRAM[0][0])  # bits 2,3,4,5 cleared


def test_SetByte_Fine_OutOfBounds():
    clear_gram()
    OLED_SetByte_Fine(10, 0, 0xFF, 0, 7, OLED_COLOR_NORMAL)
    OLED_SetByte_Fine(0, 130, 0xFF, 0, 7, OLED_COLOR_NORMAL)
    assert_equal(0, count_set_pixels())


# ===================== Test: OLED_SetBits =====================

def test_SetBits_NoPageCross():
    clear_gram()
    OLED_SetBits(0, 0, 0xAA, OLED_COLOR_NORMAL)
    assert_equal(0xAA, OLED_GRAM[0][0])


def test_SetBits_WithPageCross():
    clear_gram()
    OLED_SetBits(0, 4, 0xFF, OLED_COLOR_NORMAL)
    assert_equal(0xF0, OLED_GRAM[0][0])
    assert_equal(0x0F, OLED_GRAM[1][0])


def test_SetBits_ReversedMode():
    clear_gram()
    OLED_SetBits(0, 0, 0x0F, OLED_COLOR_REVERSED)
    assert_equal((~0x0F) & 0xFF, OLED_GRAM[0][0])


# ===================== Test: OLED_SetBits_Fine =====================

def test_SetBits_Fine_NoPageCross():
    clear_gram()
    OLED_SetBits_Fine(5, 0, 0x0F, 4, OLED_COLOR_NORMAL)
    assert_equal(0x0F, OLED_GRAM[0][5])


def test_SetBits_Fine_PageCross():
    clear_gram()
    OLED_SetBits_Fine(5, 4, 0x1F, 5, OLED_COLOR_NORMAL)
    assert_equal(0xF0, OLED_GRAM[0][5])
    assert_equal(0x01, OLED_GRAM[1][5])


# ===================== Test: OLED_SetBlock =====================

def test_SetBlock_FullBytes():
    clear_gram()
    data = [0xFF, 0x00, 0xFF, 0x00]
    OLED_SetBlock(0, 0, data, 2, 16, OLED_COLOR_NORMAL)
    assert_equal(0xFF, OLED_GRAM[0][0])
    assert_equal(0xFF, OLED_GRAM[1][0])
    assert_equal(0x00, OLED_GRAM[0][1])
    assert_equal(0x00, OLED_GRAM[1][1])


def test_SetBlock_PartialHeight():
    clear_gram()
    data = [0xFF]
    OLED_SetBlock(0, 0, data, 1, 5, OLED_COLOR_NORMAL)
    assert_equal(0x1F, OLED_GRAM[0][0])


# ===================== Test: OLED_DrawLine =====================

def test_DrawLine_Horizontal():
    clear_gram()
    OLED_DrawLine(10, 20, 50, 20, OLED_COLOR_NORMAL)
    for x in range(10, 51):
        assert_true(is_pixel_set(x, 20), f"Pixel ({x}, 20) not set")
    assert_equal(41, count_set_pixels())


def test_DrawLine_Horizontal_Reversed():
    clear_gram()
    OLED_DrawLine(50, 20, 10, 20, OLED_COLOR_NORMAL)
    assert_true(is_pixel_set(10, 20))
    assert_true(is_pixel_set(50, 20))
    assert_equal(41, count_set_pixels())


def test_DrawLine_Vertical():
    clear_gram()
    OLED_DrawLine(30, 10, 30, 40, OLED_COLOR_NORMAL)
    for y in range(10, 41):
        assert_true(is_pixel_set(30, y), f"Pixel (30, {y}) not set")
    assert_equal(31, count_set_pixels())


def test_DrawLine_Vertical_Reversed():
    clear_gram()
    OLED_DrawLine(30, 40, 30, 10, OLED_COLOR_NORMAL)
    for y in range(10, 41):
        assert_true(is_pixel_set(30, y))


def test_DrawLine_Diagonal():
    clear_gram()
    OLED_DrawLine(0, 0, 10, 10, OLED_COLOR_NORMAL)
    # Bresenham draws from x1 up to but NOT including x2 (while x != x2)
    # So endpoint (10,10) may not be drawn. But (0,0) is drawn.
    assert_true(is_pixel_set(0, 0))
    # The line should have multiple pixels
    assert_true(count_set_pixels() >= 2)


def test_DrawLine_SinglePoint():
    clear_gram()
    OLED_DrawLine(5, 5, 5, 5, OLED_COLOR_NORMAL)
    assert_true(is_pixel_set(5, 5))
    assert_equal(1, count_set_pixels())


# ===================== Test: OLED_DrawRectangle =====================

def test_DrawRectangle_Basic():
    clear_gram()
    OLED_DrawRectangle(10, 10, 20, 15, OLED_COLOR_NORMAL)
    assert_true(is_pixel_set(10, 10))
    assert_true(is_pixel_set(30, 10))
    assert_true(is_pixel_set(10, 25))
    assert_true(is_pixel_set(30, 25))
    assert_false(is_pixel_set(20, 17))


# ===================== Test: OLED_DrawFilledRectangle =====================

def test_DrawFilledRectangle_Basic():
    clear_gram()
    # C code: for (i = 0; i < h; i++) draws h lines, each w+1 pixels wide
    # w=10, h=10 -> 10 lines * 11 pixels = 110
    OLED_DrawFilledRectangle(0, 0, 10, 10, OLED_COLOR_NORMAL)
    assert_equal(110, count_set_pixels())  # 10 * (10+1)


# ===================== Test: OLED_DrawTriangle =====================

def test_DrawTriangle_Basic():
    clear_gram()
    OLED_DrawTriangle(10, 10, 50, 10, 30, 50, OLED_COLOR_NORMAL)
    assert_true(is_pixel_set(10, 10))
    assert_true(is_pixel_set(50, 10))
    assert_true(is_pixel_set(30, 50))
    assert_true(count_set_pixels() >= 3)


# ===================== Test: OLED_DrawCircle =====================

def test_DrawCircle_Basic():
    clear_gram()
    OLED_DrawCircle(64, 32, 10, OLED_COLOR_NORMAL)
    assert_true(count_set_pixels() > 0)


def test_DrawCircle_ZeroRadius():
    clear_gram()
    OLED_DrawCircle(50, 30, 0, OLED_COLOR_NORMAL)
    assert_true(count_set_pixels() >= 1)


# ===================== Test: OLED_DrawFilledCircle =====================

def test_DrawFilledCircle_Basic():
    clear_gram()
    OLED_DrawFilledCircle(64, 32, 5, OLED_COLOR_NORMAL)
    assert_true(is_pixel_set(64, 32))
    assert_true(count_set_pixels() > 0)


# ===================== Test: OLED_DrawEllipse =====================

def test_DrawEllipse_Basic():
    clear_gram()
    OLED_DrawEllipse(64, 32, 20, 10, OLED_COLOR_NORMAL)
    assert_true(count_set_pixels() > 0)


def test_DrawEllipse_CircleLike():
    clear_gram()
    OLED_DrawEllipse(50, 30, 10, 10, OLED_COLOR_NORMAL)
    assert_true(count_set_pixels() > 0)


# ===================== Test: OLED_DrawImage =====================

def test_DrawImage_Basic():
    clear_gram()
    img_data = [0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00]
    OLED_DrawImage(10, 0, img_data, 2, 16, OLED_COLOR_NORMAL)
    assert_true(count_set_pixels() > 0)


# ===================== Test: OLED_PrintASCIIChar =====================

def test_PrintASCIIChar_Basic():
    clear_gram()
    # Simple 8x6 font: space=0, '!'=has data
    char_data = [
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  # space
        0x00, 0x00, 0x00, 0x2f, 0x00, 0x00,  # !
    ]
    OLED_PrintASCIIChar(0, 0, '!', 8, 6, char_data, OLED_COLOR_NORMAL)
    assert_true(count_set_pixels() > 0)


# ===================== Test: OLED_PrintASCIIString =====================

def test_PrintASCIIString_Basic():
    clear_gram()
    char_data = [
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  # space
        0x00, 0x00, 0x00, 0x2f, 0x00, 0x00,  # !
    ]
    OLED_PrintASCIIString(0, 0, "!!", 8, 6, char_data, OLED_COLOR_NORMAL)
    assert_true(count_set_pixels() > 0)


def test_PrintASCIIString_Empty():
    clear_gram()
    char_data = [0x00, 0x00, 0x00, 0x00, 0x00, 0x00]
    OLED_PrintASCIIString(0, 0, "", 8, 6, char_data, OLED_COLOR_NORMAL)
    assert_equal(0, count_set_pixels())


# ===================== Test: _OLED_GetUTF8Len =====================

def test_GetUTF8Len_ASCII():
    assert_equal(1, _OLED_GetUTF8Len("A"))


def test_GetUTF8Len_2Byte():
    assert_equal(2, _OLED_GetUTF8Len("\u00A9"))  # (c)


def test_GetUTF8Len_3Byte():
    assert_equal(3, _OLED_GetUTF8Len("\u611F"))  # Chinese char


def test_GetUTF8Len_4Byte():
    assert_equal(4, _OLED_GetUTF8Len("\U0001F600"))  # emoji


def test_GetUTF8Len_Invalid():
    # Invalid UTF-8 start byte 0xFE
    s = bytes([0xFE])
    assert_equal(0, _OLED_GetUTF8Len(s))


# ===================== Test: OLED_DrawFilledTriangle =====================

def test_DrawFilledTriangle_Basic():
    clear_gram()
    # Triangle with vertices (10,10), (50,10), (30,50)
    OLED_DrawFilledTriangle(10, 10, 50, 10, 30, 50, OLED_COLOR_NORMAL)
    # Should draw pixels filling the triangle
    pixels = count_set_pixels()
    assert_true(pixels > 0, f"Expected some pixels, got {pixels}")


# ===================== Test Runner =====================

def main():
    global passed, failed

    print()
    print("=" * 50)
    print("  OLED Driver Unit Tests (Python port)")
    print("=" * 50)
    print()

    # OLED_SetPixel tests
    print("[OLED_SetPixel]")
    run_test("test_SetPixel_Normal_InsideBounds", test_SetPixel_Normal_InsideBounds)
    run_test("test_SetPixel_Normal_OutOfBounds", test_SetPixel_Normal_OutOfBounds)
    run_test("test_SetPixel_Reversed_ClearsPixel", test_SetPixel_Reversed_ClearsPixel)
    run_test("test_SetPixel_MultiplePixels", test_SetPixel_MultiplePixels)
    run_test("test_SetPixel_BoundaryConditions", test_SetPixel_BoundaryConditions)

    # OLED_NewFrame tests
    print("\n[OLED_NewFrame]")
    run_test("test_NewFrame_ClearsGRAM", test_NewFrame_ClearsGRAM)

    # OLED_SetByte tests
    print("\n[OLED_SetByte]")
    run_test("test_SetByte_Normal", test_SetByte_Normal)
    run_test("test_SetByte_Reversed", test_SetByte_Reversed)
    run_test("test_SetByte_OutOfBounds", test_SetByte_OutOfBounds)

    # OLED_SetByte_Fine tests
    print("\n[OLED_SetByte_Fine]")
    run_test("test_SetByte_Fine_Normal_SetMiddleBits", test_SetByte_Fine_Normal_SetMiddleBits)
    run_test("test_SetByte_Fine_Reversed_ClearsBits", test_SetByte_Fine_Reversed_ClearsBits)
    run_test("test_SetByte_Fine_OutOfBounds", test_SetByte_Fine_OutOfBounds)

    # OLED_SetBits tests
    print("\n[OLED_SetBits]")
    run_test("test_SetBits_NoPageCross", test_SetBits_NoPageCross)
    run_test("test_SetBits_WithPageCross", test_SetBits_WithPageCross)
    run_test("test_SetBits_ReversedMode", test_SetBits_ReversedMode)

    # OLED_SetBits_Fine tests
    print("\n[OLED_SetBits_Fine]")
    run_test("test_SetBits_Fine_NoPageCross", test_SetBits_Fine_NoPageCross)
    run_test("test_SetBits_Fine_PageCross", test_SetBits_Fine_PageCross)

    # OLED_SetBlock tests
    print("\n[OLED_SetBlock]")
    run_test("test_SetBlock_FullBytes", test_SetBlock_FullBytes)
    run_test("test_SetBlock_PartialHeight", test_SetBlock_PartialHeight)

    # OLED_DrawLine tests
    print("\n[OLED_DrawLine]")
    run_test("test_DrawLine_Horizontal", test_DrawLine_Horizontal)
    run_test("test_DrawLine_Horizontal_Reversed", test_DrawLine_Horizontal_Reversed)
    run_test("test_DrawLine_Vertical", test_DrawLine_Vertical)
    run_test("test_DrawLine_Vertical_Reversed", test_DrawLine_Vertical_Reversed)
    run_test("test_DrawLine_Diagonal", test_DrawLine_Diagonal)
    run_test("test_DrawLine_SinglePoint", test_DrawLine_SinglePoint)

    # OLED_DrawRectangle tests
    print("\n[OLED_DrawRectangle]")
    run_test("test_DrawRectangle_Basic", test_DrawRectangle_Basic)

    # OLED_DrawFilledRectangle tests
    print("\n[OLED_DrawFilledRectangle]")
    run_test("test_DrawFilledRectangle_Basic", test_DrawFilledRectangle_Basic)

    # OLED_DrawTriangle tests
    print("\n[OLED_DrawTriangle]")
    run_test("test_DrawTriangle_Basic", test_DrawTriangle_Basic)

    # OLED_DrawFilledTriangle tests
    print("\n[OLED_DrawFilledTriangle]")
    run_test("test_DrawFilledTriangle_Basic", test_DrawFilledTriangle_Basic)

    # OLED_DrawCircle tests
    print("\n[OLED_DrawCircle]")
    run_test("test_DrawCircle_Basic", test_DrawCircle_Basic)
    run_test("test_DrawCircle_ZeroRadius", test_DrawCircle_ZeroRadius)

    # OLED_DrawFilledCircle tests
    print("\n[OLED_DrawFilledCircle]")
    run_test("test_DrawFilledCircle_Basic", test_DrawFilledCircle_Basic)

    # OLED_DrawEllipse tests
    print("\n[OLED_DrawEllipse]")
    run_test("test_DrawEllipse_Basic", test_DrawEllipse_Basic)
    run_test("test_DrawEllipse_CircleLike", test_DrawEllipse_CircleLike)

    # OLED_DrawImage tests
    print("\n[OLED_DrawImage]")
    run_test("test_DrawImage_Basic", test_DrawImage_Basic)

    # OLED_PrintASCIIChar tests
    print("\n[OLED_PrintASCIIChar]")
    run_test("test_PrintASCIIChar_Basic", test_PrintASCIIChar_Basic)

    # OLED_PrintASCIIString tests
    print("\n[OLED_PrintASCIIString]")
    run_test("test_PrintASCIIString_Basic", test_PrintASCIIString_Basic)
    run_test("test_PrintASCIIString_Empty", test_PrintASCIIString_Empty)

    # _OLED_GetUTF8Len tests
    print("\n[_OLED_GetUTF8Len]")
    run_test("test_GetUTF8Len_ASCII", test_GetUTF8Len_ASCII)
    run_test("test_GetUTF8Len_2Byte", test_GetUTF8Len_2Byte)
    run_test("test_GetUTF8Len_3Byte", test_GetUTF8Len_3Byte)
    run_test("test_GetUTF8Len_4Byte", test_GetUTF8Len_4Byte)
    run_test("test_GetUTF8Len_Invalid", test_GetUTF8Len_Invalid)

    # Summary
    total = passed + failed
    print()
    print("=" * 50)
    print(f"  Results: {passed} passed, {failed} failed, {total} total")
    if total > 0:
        print(f"  Success rate: {passed * 100 / total:.1f}%")
    print("=" * 50)
    print()

    return 0 if failed == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
