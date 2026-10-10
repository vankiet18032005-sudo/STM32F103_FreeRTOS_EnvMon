#ifndef __GLOBAL_H
#define __GLOBAL_H

#include "main.h"
#include "cmsis_os2.h"

/* ---------- Shared data types ---------- */
typedef struct {
    float temp;
    float humi;
    uint8_t is_valid;
} SensorData_t;

/* ---------- FreeRTOS kernel objects (defined in freertos.c) ---------- */
extern osMessageQueueId_t xQueueSensorDataHandle;
extern osMessageQueueId_t xQueueKeyEventHandle;
extern osSemaphoreId_t    oledDmaSemaHandle;

/* ---------- I2C Mutex (defined in main.c) ---------- */
extern osMutexId_t i2cMutexHandle;

/* ---------- Global error counter (defined in main.c) ---------- */
extern volatile uint8_t g_dht_error_count;

/* ---------- HAL peripheral handles (defined in main.c) ---------- */
extern I2C_HandleTypeDef  hi2c1;
extern UART_HandleTypeDef huart1;
extern IWDG_HandleTypeDef hiwdg;

#endif /* __GLOBAL_H */
