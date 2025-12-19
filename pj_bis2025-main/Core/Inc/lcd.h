/* lcd.h ---------------------------------------------------------- */
#ifndef LPM013_LCD_H
#define LPM013_LCD_H
#include "main.h"

void LCD_Init(void);
void LCD_AllClear(void);
void LCD_FillWhite(void);
void LCD_SendLine4bit(uint16_t y, const uint8_t *buf);
void LCD_DrawString4bit(uint16_t y0, const char *str);
void LCD_DrawImage(void);

/* アニメーション関数 */
void LCD_ScrollText(uint16_t y0, const char *str, uint16_t scroll_offset);
void LCD_BounceText(const char *str, uint16_t bounce_y);
void LCD_BlinkText(uint16_t y0, const char *str, uint8_t visible);

#endif /* LPM013_LCD_H */