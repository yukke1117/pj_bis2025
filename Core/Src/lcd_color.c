/* lcd_color.c - カラー表示拡張関数 */
#include "lcd.h"
#include "font8x8_basic.h"
#include <string.h>

extern SPI_HandleTypeDef hspi1;

#define LCD_CS_GPIO_Port   GPIOC
#define LCD_CS_Pin         GPIO_PIN_9
#define HDR8BIT   0x20   /* 8-bit color mode header */

/* 内部ヘルパ */
static inline void LCD_SELECT  (void){ HAL_GPIO_WritePin(LCD_CS_GPIO_Port,  LCD_CS_Pin, GPIO_PIN_SET); }
static inline void LCD_UNSELECT(void){ HAL_GPIO_WritePin(LCD_CS_GPIO_Port,  LCD_CS_Pin, GPIO_PIN_RESET);   }
static inline void lcd_tx(const uint8_t *p, size_t len){
    HAL_SPI_Transmit(&hspi1, (uint8_t*)p, len, HAL_MAX_DELAY);
}

/* カラー表示用の関数群 */
void LCD_SendLineColor(uint16_t y, const uint8_t *buf)
{
    uint16_t hdr = (HDR8BIT << 10) | (y & 0x03FF);   /* 16-bit ヘッダ for 8-bit color */
    uint8_t  tx[2 + 176 + 2];                        /* hdr + data + 2B dummy */

    tx[0] = hdr >> 8;         /* MSB first */
    tx[1] = hdr & 0xFF;
    memcpy(&tx[2], buf, 176); /* 176 pixels × 1 byte each */
    tx[178] = tx[179] = 0x00; /* 16clk transfer period */

    LCD_SELECT();
    lcd_tx(tx, sizeof tx);
    LCD_UNSELECT();
}

void LCD_FillColor(uint8_t color)
{
    uint8_t rowbuf[176];
    memset(rowbuf, color, sizeof rowbuf);
    
    for (uint16_t y = 0; y < 176; y++) {
        LCD_SendLineColor(y, rowbuf);
    }
}

void LCD_DrawStringColor(uint16_t y0, const char *str, uint8_t color)
{
    const uint16_t len   = strlen(str);
    const uint16_t xmax  = len * 8;          /* 文字幅(ピクセル) */
    uint8_t rowbuf[176];                     /* 176px = 176 bytes for color */

    for (uint8_t font_row = 0; font_row < 8; ++font_row) {
        /* 1 行分ゼロクリア */
        memset(rowbuf, COLOR_BLACK, sizeof rowbuf);

        /* 文字→ピクセル→8-bit color */
        for (uint16_t x = 0; x < xmax && x < 176; ++x) {
            uint8_t ch   = str[x / 8] - 32;      /* ASCII32=space 基点 */
            uint8_t bit  = (font8x8_basic[ch][font_row] >> (x % 8)) & 1;
            
            if (bit) {
                rowbuf[x] = color;
            }
        }
        /* 画面に送る */
        LCD_SendLineColor(y0 + font_row, rowbuf);
    }
}

void LCD_DrawImageColor(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t *image_data)
{
    uint8_t rowbuf[176];
    
    for (uint16_t row = 0; row < height; row++) {
        if (y + row >= 176) break; /* 画面外チェック */
        
        /* 行全体をクリア */
        memset(rowbuf, COLOR_BLACK, sizeof rowbuf);
        
        /* 画像データをコピー */
        for (uint16_t col = 0; col < width; col++) {
            if (x + col >= 176) break; /* 画面外チェック */
            
            uint16_t img_idx = row * width + col;
            rowbuf[x + col] = image_data[img_idx];
        }
        
        LCD_SendLineColor(y + row, rowbuf);
    }
}