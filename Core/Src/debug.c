/**
  ******************************************************************************
  * @file           : debug.c
  * @brief          : UART debug log output and printf redirection
  ******************************************************************************
  */

#include "debug.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

extern UART_HandleTypeDef huart1;

/**
  * @brief  Transmit formatted debug string over UART1
  * @param  format: Format string
  * @param  ...: Variable arguments
  */
void DEBUG_Log(const char *format, ...)
{
    char buffer[128];
    va_list args;

    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    HAL_UART_Transmit(&huart1, (uint8_t *)buffer, (uint16_t)strlen(buffer), 100);
}

/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  ch: Character to send
  * @retval Character sent
  */
int __io_putchar(int ch)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 10);
    return ch;
}
