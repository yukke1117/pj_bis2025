#include "lcd.h"
#include "font8x8_basic.h"
#include "main.h"
#include <string.h>

#define PIX_ON   0b1110   /* RGB=111, D=0 → 白 */
#define PIX_OFF  0b0000   /* 黒 */

/* 文字列を y0 行から 8 行ぶん描画（4‑bit data mode） */
void LCD_DrawString4bit(uint16_t y0, const char *str)
{
    const uint16_t len   = strlen(str);
    const uint16_t xmax  = len * 8;          /* 文字幅(ピクセル) */
    uint8_t rowbuf[88];                      /* 176px ÷2 = 88B */

    for (uint8_t font_row = 0; font_row < 8; ++font_row) {
        /* 1 行分ゼロクリア */
        memset(rowbuf, 0, sizeof rowbuf);

        /* 文字→ピクセル→4‑bit パック */
        for (uint16_t x = 0; x < xmax; ++x) {
            uint8_t ch   = str[x / 8] - 32;      /* ASCII32=space 基点 */
            uint8_t bit  = (font8x8_basic[ch][font_row] >> (x % 8)) & 1;
            uint8_t pix4 = bit ? PIX_ON : PIX_OFF;

            /* 2 ピクセルで 1 byte：偶数 x を上位 4bit、奇数 x を下位 4bit に */
            uint16_t byte_idx = x / 2;
            if (x & 1)
                rowbuf[byte_idx]  |= pix4;           /* 下位 nibble */
            else
                rowbuf[byte_idx]  |= pix4 << 4;      /* 上位 nibble */
        }
        /* 画面に送る（4‑bit data mode ヘッダ + dummy 16clk は関数側で付加）*/
        LCD_SendLine4bit(y0 + font_row, rowbuf);
    }
}

/* 横スクロールアニメーション */
void LCD_ScrollText(uint16_t y0, const char *str, uint16_t scroll_offset)
{
    const uint16_t len = strlen(str);
    const uint16_t text_width = len * 8;    /* 文字列の総幅（ピクセル） */
    const uint16_t screen_width = 176;      /* 画面幅 */
    uint8_t rowbuf[88];                     /* 176px ÷2 = 88B */

    for (uint8_t font_row = 0; font_row < 8; ++font_row) {
        /* 1 行分ゼロクリア */
        memset(rowbuf, 0, sizeof rowbuf);

        /* 画面の各ピクセル位置について */
        for (uint16_t screen_x = 0; screen_x < screen_width; ++screen_x) {
            /* スクロールオフセットを考慮した文字列内の位置 */
            int16_t text_x = screen_x + scroll_offset;
            
            uint8_t pix4 = PIX_OFF;  /* デフォルトは黒 */
            
            /* 文字列の範囲内かチェック */
            if (text_x >= 0 && text_x < text_width) {
                uint8_t ch = str[text_x / 8] - 32;      /* ASCII32=space 基点 */
                uint8_t bit = (font8x8_basic[ch][font_row] >> (7 - (text_x % 8))) & 1;
                pix4 = bit ? PIX_ON : PIX_OFF;
            }

            /* 2 ピクセルで 1 byte：偶数 x を上位 4bit、奇数 x を下位 4bit に */
            uint16_t byte_idx = screen_x / 2;
            if (screen_x & 1)
                rowbuf[byte_idx] |= pix4;           /* 下位 nibble */
            else
                rowbuf[byte_idx] |= pix4 << 4;      /* 上位 nibble */
        }
        /* 画面に送る */
        LCD_SendLine4bit(y0 + font_row, rowbuf);
    }
}

/* 上下バウンスアニメーション */
void LCD_BounceText(const char *str, uint16_t bounce_y)
{
	LCD_AllClear();
    /* 指定されたY位置に文字列を描画 */
    LCD_DrawString4bit(bounce_y, str);
    HAL_Delay(30); // 30ms待つ
}

/* 点滅アニメーション */
void LCD_BlinkText(uint16_t y0, const char *str, uint8_t visible)
{
    if (visible) {
        LCD_DrawString4bit(y0, str);
    } else {
        /* 該当行をクリア */
        uint8_t rowbuf[88];
        memset(rowbuf, 0, sizeof rowbuf);
        for (uint8_t font_row = 0; font_row < 8; ++font_row) {
            LCD_SendLine4bit(y0 + font_row, rowbuf);
        }
    }
}
