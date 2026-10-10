#include "oled.h"
#include "oled_font.h"
#include "global.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os2.h"
#include <string.h>
#include <stdio.h>

/* ---------- Global data ---------- */
uint8_t framebuffer[1024];
static uint8_t dma_tx_buf[2][1025];
static uint8_t buf_idx = 0;
static volatile uint8_t is_dma_busy = 0;

/* ---------- Externs ---------- */
extern I2C_HandleTypeDef hi2c1;
extern osSemaphoreId_t oledDmaSemaHandle;
extern osMutexId_t i2cMutexHandle;

/* ---------- Private SSD1306 Initialization Commands ---------- */
static const uint8_t oled_init_sequence[] = {
    0x00,             /* Control byte: Co = 0, D/C# = 0 (stream of commands) */
    0xAE,             /* Display OFF */
    0xD5, 0x80,       /* Set Display Clock Divide Ratio / Oscillator Frequency */
    0xA8, 0x3F,       /* Set Multiplex Ratio (64MUX for 128x64) */
    0xD3, 0x00,       /* Set Display Offset to 0 */
    0x40,             /* Set Display Start Line to 0 */
    0x8D, 0x14,       /* Charge Pump Setting: Enable charge pump */
    0x20, 0x00,       /* Set Memory Addressing Mode: Horizontal addressing mode */
    0xA1,             /* Set Segment Re-map: Column 127 mapped to SEG0 */
    0xC8,             /* Set COM Output Scan Direction: Remapped mode */
    0xDA, 0x12,       /* Set COM Pins Hardware Configuration: Alternative COM config */
    0x81, 0xCF,       /* Set Contrast Control to 0xCF */
    0xD9, 0xF1,       /* Set Pre-charge Period */
    0xDB, 0x40,       /* Set VCOMH Deselect Level */
    0xA4,             /* Entire Display ON: Resume to RAM content */
    0xA6,             /* Set Normal Display (not inverse) */
    0x21, 0x00, 0x7F, /* Set Column Address Range: start = 0, end = 127 */
    0x22, 0x00, 0x07, /* Set Page Address Range: start = 0, end = 7 */
    0xAF              /* Display ON */
};

/**
  * @brief  HAL I2C Master Tx Complete callback.
  *         Invoked upon completion of DMA transmit.
  * @param  hi2c: Pointer to I2C handle
  * @retval None
  */
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == I2C1) {
        is_dma_busy = 0;
        osSemaphoreRelease(oledDmaSemaHandle);
    }
}

/**
  * @brief  HAL I2C Error callback.
  *         Invoked if an error occurs during DMA transmit.
  * @param  hi2c: Pointer to I2C handle
  * @retval None
  */
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == I2C1) {
        is_dma_busy = 0;
        osSemaphoreRelease(oledDmaSemaHandle);
    }
}

/**
  * @brief  Send a single command byte via blocking I2C.
  * @param  cmd: SSD1306 command byte
  * @retval None
  */
static void OLED_WriteCmd(uint8_t cmd)
{
    uint8_t buf[2];
    buf[0] = 0x00;  /* Control byte: command mode */
    buf[1] = cmd;
    HAL_I2C_Master_Transmit(&hi2c1, OLED_ADDRESS, buf, 2, 100);
}

/**
  * @brief  Initialize SSD1306 OLED display (128x64).
  * @retval None
  */
void OLED_Init(void)
{
    /* Wait 100ms for power stabilization */
    osDelay(100);

    /* Send entire init sequence in a single I2C blocking transaction */
    HAL_I2C_Master_Transmit(&hi2c1, OLED_ADDRESS, (uint8_t *)oled_init_sequence, sizeof(oled_init_sequence), 100);

    /* Clear framebuffer and push initial frame via DMA */
    OLED_Clear();
    OLED_Update_DMA_SingleFrame();
}

/**
  * @brief  Clear 1024-byte framebuffer.
  * @retval None
  */
void OLED_Clear(void)
{
    memset(framebuffer, 0, sizeof(framebuffer));
}

/**
  * @brief  Send framebuffer to SSD1306 via DMA using double buffering.
  * @retval None
  */
void OLED_Update_DMA_SingleFrame(void)
{
    taskENTER_CRITICAL();
    if (is_dma_busy) {
        taskEXIT_CRITICAL();
        return;
    }
    is_dma_busy = 1;
    taskEXIT_CRITICAL();

    buf_idx ^= 1;
    dma_tx_buf[buf_idx][0] = 0x40;  /* Data mode control byte */
    memcpy(&dma_tx_buf[buf_idx][1], framebuffer, 1024);

    osMutexAcquire(i2cMutexHandle, osWaitForever);
    if (HAL_I2C_Master_Transmit_DMA(&hi2c1, OLED_ADDRESS, dma_tx_buf[buf_idx], 1025) == HAL_OK) {
        osSemaphoreAcquire(oledDmaSemaHandle, osWaitForever);
    } else {
        is_dma_busy = 0;
    }
    osMutexRelease(i2cMutexHandle);
}

/**
  * @brief  Render a single character at specified (x, page) position.
  * @param  x: Column index (0..127)
  * @param  page: Page index (0..7)
  * @param  ch: ASCII character (32..126)
  * @retval None
  */
void OLED_ShowChar(uint8_t x, uint8_t page, char ch)
{
    if (page >= 8 || x >= 128) {
        return;
    }

    if (ch < 32 || ch > 126) {
        ch = ' ';
    }

    uint8_t font_idx = (uint8_t)(ch - 32);
    uint16_t base_idx = (uint16_t)page * 128 + x;

    for (uint8_t i = 0; i < 6; i++) {
        if ((x + i) < 128) {
            framebuffer[base_idx + i] = Font_6x8[font_idx][i];
        }
    }
}

/**
  * @brief  Render a null-terminated string starting at (x, page).
  *         Advances x by 6 per char. Wraps to x=0, page++ if x > 122.
  * @param  x: Starting column index (0..127)
  * @param  page: Starting page index (0..7)
  * @param  str: Null-terminated string
  * @retval None
  */
void OLED_ShowString(uint8_t x, uint8_t page, const char *str)
{
    if (str == NULL) {
        return;
    }

    while (*str != '\0') {
        if (page > 7) {
            break;
        }

        OLED_ShowChar(x, page, *str);
        x += 6;
        if (x > 122) {
            x = 0;
            page++;
        }
        str++;
    }
}

/**
  * @brief  Format and display a floating-point number using pure integer math.
  *         Avoids linking %f / software float printf overhead.
  * @param  x: Column index (0..127)
  * @param  page: Page index (0..7)
  * @param  val: Floating-point number to display
  * @param  decimals: Number of decimal places
  * @retval None
  */
void OLED_ShowFloat(uint8_t x, uint8_t page, float val, uint8_t decimals)
{
    char buf[32];
    char *p = buf;

    if (decimals > 6) {
        decimals = 6;
    }

    /* Handle negative sign manually */
    if (val < 0.0f) {
        *p++ = '-';
        val = -val;
    }

    /* Integer part */
    int32_t whole = (int32_t)val;
    float remainder = val - (float)whole;

    /* Multiplier for fractional part: 10^decimals */
    int32_t mult = 1;
    for (uint8_t i = 0; i < decimals; i++) {
        mult *= 10;
    }

    /* Fractional part with rounding */
    int32_t frac = (int32_t)(remainder * (float)mult + 0.5f);
    if (frac >= mult) {
        frac -= mult;
        whole += 1;
    }

    /* Format string using only integer specifiers */
    if (decimals == 0) {
        snprintf(p, sizeof(buf) - (size_t)(p - buf), "%d", (int)whole);
    } else {
        snprintf(p, sizeof(buf) - (size_t)(p - buf), "%d.%0*d", (int)whole, (int)decimals, (int)frac);
    }

    OLED_ShowString(x, page, buf);
}
