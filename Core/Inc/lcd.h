/* lcd.h ---------------------------------------------------------- */
#ifndef LPM013_LCD_H
#define LPM013_LCD_H
#include "main.h"

void LCD_Init(void);
void LCD_AllClear(void);
void LCD_SendLine4bit(uint16_t y, const uint8_t *buf);
void LCD_DrawString4bit(uint16_t y0, const char *str);

/* カラー表示関数 */
void LCD_SendLineColor(uint16_t y, const uint8_t *buf);
void LCD_DrawStringColor(uint16_t y0, const char *str, uint8_t color);
void LCD_FillColor(uint8_t color);
void LCD_DrawImageColor(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t *image_data);

/* アニメーション関数 */
void LCD_ScrollText(uint16_t y0, const char *str, uint16_t scroll_offset);
void LCD_BounceText(const char *str, uint16_t bounce_y);
void LCD_BlinkText(uint16_t y0, const char *str, uint8_t visible);

/* カラー定数 */
#define COLOR_BLACK   0x00  // RGB=000, D=0
#define COLOR_RED     0x08  // RGB=100, D=0  
#define COLOR_GREEN   0x04  // RGB=010, D=0
#define COLOR_BLUE    0x02  // RGB=001, D=0
#define COLOR_YELLOW  0x0C  // RGB=110, D=0
#define COLOR_MAGENTA 0x0A  // RGB=101, D=0
#define COLOR_CYAN    0x06  // RGB=011, D=0
#define COLOR_WHITE   0x0E  // RGB=111, D=0

/* テスト用画像データ */
extern const uint8_t test_color_image_32x32[];

#endif /* LPM013_LCD_H */