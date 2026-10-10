/**
  ******************************************************************************
  * @file           : tasks.c
  * @brief          : FreeRTOS task implementations for the Environmental
  *                   Monitoring System
  ******************************************************************************
  */

#include "tasks.h"
#include "global.h"
#include "dht11.h"
#include "oled.h"
#include "key.h"
#include "led.h"
#include "debug.h"

#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os2.h"

#include <stdio.h>

/* ========================= Task_DHT ========================= */
/* Reads DHT11 sensor every 1.5s, pushes data into Queue.       */
/* If queue is full, discards oldest entry and re-inserts.       */
/* ============================================================ */
void StartTaskDHT(void *argument)
{
    (void)argument;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    SensorData_t sensor_data;

    DEBUG_Log("[INFO] Task_DHT started\r\n");

    for (;;)
    {
        sensor_data.is_valid = DHT11_ReadData(&sensor_data.temp, &sensor_data.humi);

        if (sensor_data.is_valid) {
            DEBUG_Log("[DHT] T=%d.%d H=%d.%d\r\n",
                      (int)sensor_data.temp,
                      (int)((sensor_data.temp - (int)sensor_data.temp) * 10),
                      (int)sensor_data.humi,
                      (int)((sensor_data.humi - (int)sensor_data.humi) * 10));
        }

        /* Push to queue. If full, discard oldest and retry. */
        if (osMessageQueuePut(xQueueSensorDataHandle, &sensor_data, 0, pdMS_TO_TICKS(10)) != osOK)
        {
            SensorData_t dummy;
            osMessageQueueGet(xQueueSensorDataHandle, &dummy, NULL, 0);
            osMessageQueuePut(xQueueSensorDataHandle, &sensor_data, 0, 0);
        }

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1500));
    }
}

/* ========================= Task_OLED ========================= */
/* Receives sensor data from Queue and displays on SSD1306 OLED  */
/* every 500ms.                                                  */
/* ============================================================= */
void StartTaskOLED(void *argument)
{
    (void)argument;
    TickType_t xLastWakeTime;
    SensorData_t rx_data;
    char line_buf[22];

    DEBUG_Log("[INFO] Task_OLED started\r\n");

    /* Initialize OLED (must be called after scheduler starts because it uses osDelay) */
    OLED_Init();

    xLastWakeTime = xTaskGetTickCount();

    for (;;)
    {
        /* Try to get latest sensor data (non-blocking) */
        if (osMessageQueueGet(xQueueSensorDataHandle, &rx_data, NULL, 0) == osOK)
        {
            OLED_Clear();

            if (rx_data.is_valid)
            {
                /* Line 0: Temperature */
                int t_int = (int)rx_data.temp;
                int t_dec = (int)((rx_data.temp - (float)t_int) * 10);
                if (t_dec < 0) t_dec = -t_dec;
                snprintf(line_buf, sizeof(line_buf), "Temp: %d.%d C", t_int, t_dec);
                OLED_ShowString(0, 0, line_buf);

                /* Line 2: Humidity */
                int h_int = (int)rx_data.humi;
                int h_dec = (int)((rx_data.humi - (float)h_int) * 10);
                if (h_dec < 0) h_dec = -h_dec;
                snprintf(line_buf, sizeof(line_buf), "Humi: %d.%d %%", h_int, h_dec);
                OLED_ShowString(0, 2, line_buf);

                /* Line 4: Status */
                OLED_ShowString(0, 4, "Status: OK");
            }
            else
            {
                OLED_ShowString(0, 0, "DHT Error!");
                OLED_ShowString(0, 2, "Check sensor");
            }
        }

        OLED_Update_DMA_SingleFrame();
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(500));
    }
}

/* ========================= Task_Key ========================= */
/* Scans push buttons every 20ms with debounce.                 */
/* ============================================================ */
void StartTaskKey(void *argument)
{
    (void)argument;
    TickType_t xLastWakeTime = xTaskGetTickCount();

    for (;;)
    {
        KEY_Scan();
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(20));
    }
}

/* ========================= Task_LED ========================= */
/* Heartbeat LED (green) blinks at 1Hz (toggle every 500ms).   */
/* ============================================================ */
void StartTaskLED(void *argument)
{
    (void)argument;
    TickType_t xLastWakeTime = xTaskGetTickCount();

    LED_Init();

    for (;;)
    {
        LED_Green_Toggle();
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(500));
    }
}

/* ========================= Task_Error ======================== */
/* Monitors DHT11 error count. Feeds IWDG watchdog every 1s.    */
/* If errors exceed threshold, triggers system reset.            */
/* ============================================================= */
void StartTaskError(void *argument)
{
    (void)argument;
    TickType_t xLastWakeTime = xTaskGetTickCount();

    DEBUG_Log("[INFO] Task_Error started\r\n");

    for (;;)
    {
        /* Check for persistent DHT11 failures */
        if (g_dht_error_count >= 5)
        {
            DEBUG_Log("[ERROR] DHT fail > 5 times, system reset!\r\n");
            LED_Red_On();
            HAL_Delay(100);  /* Brief visual indication before reset */
            NVIC_SystemReset();
        }

        /* Visual error indication */
        if (g_dht_error_count > 0)
        {
            LED_Red_Toggle();
        }
        else
        {
            LED_Red_Off();
        }

        /* Feed the Independent Watchdog (IWDG timeout = 2s) */
        HAL_IWDG_Refresh(&hiwdg);

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000));
    }
}
