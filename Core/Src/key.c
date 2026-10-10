/**
  ******************************************************************************
  * @file           : key.c
  * @brief          : Push button scanning driver with debounce
  ******************************************************************************
  */

#include "key.h"
#include "global.h"
#include "cmsis_os2.h"

extern osMessageQueueId_t xQueueKeyEventHandle;

static uint8_t key1_state = 0;
static uint8_t key2_state = 0;
static uint8_t key1_debounce = 0;
static uint8_t key2_debounce = 0;

/**
  * @brief  Scan push buttons PB12 and PB13 with software debouncing.
  *         Pushes key events to xQueueKeyEventHandle on state transition.
  */
void KEY_Scan(void)
{
    /* Key 1 on PB12 (active LOW, pull-up) */
    if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12) == GPIO_PIN_RESET) {
        if (key1_debounce < 3) {
            key1_debounce++;
        }
        if (key1_debounce >= 3 && key1_state == 0) {
            key1_state = 1;
            KeyEvent_t event = KEY1_PRESSED;
            if (xQueueKeyEventHandle != NULL) {
                osMessageQueuePut(xQueueKeyEventHandle, &event, 0U, 0U);
            }
        }
    } else {
        key1_debounce = 0;
        key1_state = 0;
    }

    /* Key 2 on PB13 (active LOW, pull-up) */
    if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13) == GPIO_PIN_RESET) {
        if (key2_debounce < 3) {
            key2_debounce++;
        }
        if (key2_debounce >= 3 && key2_state == 0) {
            key2_state = 1;
            KeyEvent_t event = KEY2_PRESSED;
            if (xQueueKeyEventHandle != NULL) {
                osMessageQueuePut(xQueueKeyEventHandle, &event, 0U, 0U);
            }
        }
    } else {
        key2_debounce = 0;
        key2_state = 0;
    }
}
