/* lcd.c ---------------------------------------------------------- */
#include "lcd.h"
#include <string.h>

extern SPI_HandleTypeDef hspi1;          /* CubeMX が生成したハンドル */

#define LCD_CS_GPIO_Port   GPIOC
#define LCD_CS_Pin         GPIO_PIN_9
#define LCD_DISP_GPIO_Port GPIOC
#define LCD_DISP_Pin       GPIO_PIN_10
#define HDR4BIT   0x24
#define HDR8BIT   0x20   /* 8-bit color mode header */

/* 内部ヘルパ */
static inline void LCD_SELECT  (void){ HAL_GPIO_WritePin(LCD_CS_GPIO_Port,  LCD_CS_Pin, GPIO_PIN_SET); }
static inline void LCD_UNSELECT(void){ HAL_GPIO_WritePin(LCD_CS_GPIO_Port,  LCD_CS_Pin, GPIO_PIN_RESET);   }
static inline void lcd_tx(const uint8_t *p, size_t len){
    HAL_SPI_Transmit(&hspi1, (uint8_t*)p, len, HAL_MAX_DELAY);
}

/* --- Public API ------------------------------------------------- */
void LCD_Init(void)
{
    /* ハードリセット相当 */
    HAL_GPIO_WritePin(LCD_DISP_GPIO_Port, LCD_DISP_Pin, GPIO_PIN_RESET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(LCD_DISP_GPIO_Port, LCD_DISP_Pin, GPIO_PIN_SET);
    HAL_Delay(10);

    LCD_AllClear();                      /* メモリ全消去 (CMD 0x20) */
}

void LCD_AllClear(void)
{
    const uint8_t cmd = 0x20;            /* ALL CLEAR */
    LCD_SELECT();
    lcd_tx(&cmd, 1);
    LCD_UNSELECT();
}

void LCD_SendLine4bit(uint16_t y, const uint8_t *buf)
{
    uint16_t hdr = (HDR4BIT << 10) | (y & 0x03FF);   /* 16‑bit ヘッダ */
    uint8_t  tx[2 + 88 + 2];                         /* hdr + data + 2B dummy */

    tx[0] = hdr >> 8;         /* MSB first */
    tx[1] = hdr & 0xFF;
    memcpy(&tx[2], buf, 88);
    tx[90] = tx[91] = 0x00;   /* 16clk transfer period */

    LCD_SELECT();             /* ★ CS = High (SCS=H がアクティブ) */
    lcd_tx(tx, sizeof tx);    /* SPI 8‑bit, ≒20 MHz, MSB first */
    LCD_UNSELECT();           /* CS = Low */
}
