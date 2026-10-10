/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os2.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "global.h"
#include "tasks.h"
#include "debug.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
osMessageQueueId_t xQueueSensorDataHandle;
osMessageQueueId_t xQueueKeyEventHandle;
osSemaphoreId_t    oledDmaSemaHandle;
/* USER CODE END Variables */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

/* ---- Hook functions required by configCHECK_FOR_STACK_OVERFLOW and configUSE_MALLOC_FAILED_HOOK ---- */

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
  (void)xTask;
  DEBUG_Log("[FATAL] Stack overflow in task: %s\r\n", pcTaskName);
  Error_Handler();
}

void vApplicationMallocFailedHook(void)
{
  DEBUG_Log("[FATAL] Malloc failed!\r\n");
  Error_Handler();
}

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void)
{
  /* USER CODE BEGIN RTOS_Init */

  /* Create Queues */
  xQueueSensorDataHandle = osMessageQueueNew(5, sizeof(SensorData_t), NULL);
  xQueueKeyEventHandle   = osMessageQueueNew(10, sizeof(uint8_t), NULL);

  /* Create Semaphore for OLED DMA complete notification (initial count = 0) */
  oledDmaSemaHandle = osSemaphoreNew(1, 0, NULL);

  /* Task attributes */
  const osThreadAttr_t attr_DHT = {
      .name = "TaskDHT",
      .stack_size = 256 * 4,
      .priority = (osPriority_t) osPriorityNormal,
  };
  const osThreadAttr_t attr_OLED = {
      .name = "TaskOLED",
      .stack_size = 512 * 4,
      .priority = (osPriority_t) osPriorityNormal,
  };
  const osThreadAttr_t attr_Key = {
      .name = "TaskKey",
      .stack_size = 128 * 4,
      .priority = (osPriority_t) osPriorityAboveNormal,
  };
  const osThreadAttr_t attr_LED = {
      .name = "TaskLED",
      .stack_size = 128 * 4,
      .priority = (osPriority_t) osPriorityBelowNormal,
  };
  const osThreadAttr_t attr_Error = {
      .name = "TaskError",
      .stack_size = 256 * 4,
      .priority = (osPriority_t) osPriorityBelowNormal,
  };

  /* Create Tasks */
  osThreadNew(StartTaskDHT,   NULL, &attr_DHT);
  osThreadNew(StartTaskOLED,  NULL, &attr_OLED);
  osThreadNew(StartTaskKey,   NULL, &attr_Key);
  osThreadNew(StartTaskLED,   NULL, &attr_LED);
  osThreadNew(StartTaskError, NULL, &attr_Error);

  /* USER CODE END RTOS_Init */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* Idle task requires static memory when configSUPPORT_STATIC_ALLOCATION is 1 */
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize)
{
  static StaticTask_t xIdleTaskTCB;
  static StackType_t  uxIdleTaskStack[configMINIMAL_STACK_SIZE];

  *ppxIdleTaskTCBBuffer   = &xIdleTaskTCB;
  *ppxIdleTaskStackBuffer = uxIdleTaskStack;
  *pulIdleTaskStackSize   = configMINIMAL_STACK_SIZE;
}

void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                    StackType_t **ppxTimerTaskStackBuffer,
                                    uint32_t *pulTimerTaskStackSize)
{
  static StaticTask_t xTimerTaskTCB;
  static StackType_t  uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH];

  *ppxTimerTaskTCBBuffer   = &xTimerTaskTCB;
  *ppxTimerTaskStackBuffer = uxTimerTaskStack;
  *pulTimerTaskStackSize   = configTIMER_TASK_STACK_DEPTH;
}

/* USER CODE END Application */
