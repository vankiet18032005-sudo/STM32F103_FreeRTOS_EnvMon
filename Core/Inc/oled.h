#ifndef __OLED_H
#define __OLED_H

#include "main.h"

#define OLED_ADDRESS  0x78
#define OLED_WIDTH    128
#define OLED_HEIGHT   64

void OLED_Init(void);
void OLED_Clear(void);
void OLED_Update_DMA_SingleFrame(void);
void OLED_ShowChar(uint8_t x, uint8_t page, char ch);
void OLED_ShowString(uint8_t x, uint8_t page, const char *str);
void OLED_ShowFloat(uint8_t x, uint8_t page, float val, uint8_t decimals);

extern uint8_t framebuffer[1024];

#endif /* __OLED_H */
