#include "lcd.h"
#include "font8x8_basic.h"
#include "main.h"
#include <string.h>
#include "image_logo.h"

#define PIX_ON   0b0000   /* 黒 → 文字色 */
#define PIX_OFF  0b1110   /* RGB=111, D=0 → 白 → 背景色 */

/* 文字列を y0 行から 8 行ぶん描画（4‑bit data mode） */
void LCD_DrawString4bit(uint16_t y0, const char *str)
{
    const uint16_t len   = strlen(str);
    const uint16_t xmax  = len * 8;          /* 文字幅(ピクセル) */
    uint8_t rowbuf[88];                      /* 176px ÷2 = 88B */

    for (uint8_t font_row = 0; font_row < 8; ++font_row) {
        /* 1 行分を白で初期化 */
        for (int i = 0; i < 88; i++) {
            rowbuf[i] = 0xEE;  /* 0b11101110 = 白白 */
        }

        /* 文字→ピクセル→4‑bit パック */
        for (uint16_t x = 0; x < xmax; ++x) {
            uint8_t ch   = str[x / 8] - 32;      /* ASCII32=space 基点 */
            uint8_t bit  = (font8x8_basic[ch][font_row] >> (x % 8)) & 1;
            
            /* 文字の部分のみ黒で上書き */
            if (bit) {
                uint8_t pix4 = PIX_ON;  /* 黒 */
                uint16_t byte_idx = x / 2;
                if (x & 1)
                    rowbuf[byte_idx] = (rowbuf[byte_idx] & 0xF0) | pix4;      /* 下位 nibble */
                else
                    rowbuf[byte_idx] = (rowbuf[byte_idx] & 0x0F) | (pix4 << 4); /* 上位 nibble */
            }
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
        /* 1 行分を白で初期化 */
        for (int i = 0; i < 88; i++) {
            rowbuf[i] = 0xEE;  /* 0b11101110 = 白白 */
        }

        /* 画面の各ピクセル位置について */
        for (uint16_t screen_x = 0; screen_x < screen_width; ++screen_x) {
            /* スクロールオフセットを考慮した文字列内の位置 */
            int16_t text_x = screen_x + scroll_offset;
            
            /* 文字列の範囲内かチェック */
            if (text_x >= 0 && text_x < text_width) {
                uint8_t ch = str[text_x / 8] - 32;      /* ASCII32=space 基点 */
                uint8_t bit = (font8x8_basic[ch][font_row] >> (7 - (text_x % 8))) & 1;
                
                /* 文字の部分のみ黒で上書き */
                if (bit) {
                    uint8_t pix4 = PIX_ON;  /* 黒 */
                    uint16_t byte_idx = screen_x / 2;
                    if (screen_x & 1)
                        rowbuf[byte_idx] = (rowbuf[byte_idx] & 0xF0) | pix4;      /* 下位 nibble */
                    else
                        rowbuf[byte_idx] = (rowbuf[byte_idx] & 0x0F) | (pix4 << 4); /* 上位 nibble */
                }
            }
        }
        /* 画面に送る */
        LCD_SendLine4bit(y0 + font_row, rowbuf);
    }
}

/* 上下バウンスアニメーション */
void LCD_BounceText(const char *str, uint16_t bounce_y)
{
    /* 画面全体を白で塗りつぶし */
    LCD_FillWhite();
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
        /* 該当行を白でクリア */
        uint8_t rowbuf[88];
        /* 全ピクセルを白で初期化 */
        for (int i = 0; i < 88; i++) {
            rowbuf[i] = 0xEE;  /* 0b11101110 = 白白 */
        }
        for (uint8_t font_row = 0; font_row < 8; ++font_row) {
            LCD_SendLine4bit(y0 + font_row, rowbuf);
        }
    }
}

void LCD_DrawImage(void)
{
    // 176px / 2 pixels_per_byte = 88 bytes （1行ぶん）
    uint8_t rowbuf[88];

    const uint8_t bytes_per_pixel = 3;
    const uint16_t bytes_per_row = Image_logo.width * bytes_per_pixel;

    for (uint16_t y = 0; y < Image_logo.height; y++) {
        memset(rowbuf, 0, sizeof(rowbuf));

        const uint8_t *p_src_row = &Image_logo.data[y * bytes_per_row];

        for (uint16_t x = 0; x < Image_logo.width; x++) {
            const uint8_t *p_pixel = &p_src_row[x * bytes_per_pixel];
            uint8_t r = (p_pixel[0] > 127) ? 1 : 0;
            uint8_t g = (p_pixel[1] > 127) ? 1 : 0;
            uint8_t b = (p_pixel[2] > 127) ? 1 : 0;

            // R,G,B,Dummy を 4bit にパック
            uint8_t pix4 = (r << 3) | (g << 2) | (b << 1) | 0;

            // 偶数xは上位4bit, 奇数xは下位4bit
            uint16_t byte_idx = x / 2;
            if (x & 1) {
                rowbuf[byte_idx] |= pix4 & 0x0F;
            } else {
                rowbuf[byte_idx] |= (pix4 << 4) & 0xF0;
            }
        }

        // 1ライン転送
        LCD_SendLine4bit(y, rowbuf);
    }
}

