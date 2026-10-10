/**
  ******************************************************************************
  * @file           : led.c
  * @brief          : LED driver (PC13 Red, PB14 Green)
  ******************************************************************************
  */

#include "led.h"

#define LED_RED_PORT    GPIOC
#define LED_RED_PIN     GPIO_PIN_13

#define LED_GREEN_PORT  GPIOB
#define LED_GREEN_PIN   GPIO_PIN_14

/**
  * @brief  Initialize LED GPIO pins and set default states (both OFF)
  *         PC13: Red LED (Active LOW - SET=OFF, RESET=ON)
  *         PB14: Green LED (Active HIGH - SET=ON, RESET=OFF)
  */
void LED_Init(void)
{
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Configure PC13 (Red LED, active LOW, default OFF) */
    HAL_GPIO_WritePin(LED_RED_PORT, LED_RED_PIN, GPIO_PIN_SET);
    GPIO_InitStruct.Pin = LED_RED_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_RED_PORT, &GPIO_InitStruct);

    /* Configure PB14 (Green LED, active HIGH, default OFF) */
    HAL_GPIO_WritePin(LED_GREEN_PORT, LED_GREEN_PIN, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = LED_GREEN_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_GREEN_PORT, &GPIO_InitStruct);
}

/**
  * @brief  Toggle Green LED (PB14)
  */
void LED_Green_Toggle(void)
{
    HAL_GPIO_TogglePin(LED_GREEN_PORT, LED_GREEN_PIN);
}

/**
  * @brief  Toggle Red LED (PC13)
  */
void LED_Red_Toggle(void)
{
    HAL_GPIO_TogglePin(LED_RED_PORT, LED_RED_PIN);
}

/**
  * @brief  Turn Red LED ON (active LOW: RESET)
  */
void LED_Red_On(void)
{
    HAL_GPIO_WritePin(LED_RED_PORT, LED_RED_PIN, GPIO_PIN_RESET);
}

/**
  * @brief  Turn Red LED OFF (active LOW: SET)
  */
void LED_Red_Off(void)
{
    HAL_GPIO_WritePin(LED_RED_PORT, LED_RED_PIN, GPIO_PIN_SET);
}
