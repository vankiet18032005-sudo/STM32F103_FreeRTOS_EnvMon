/**
  ******************************************************************************
  * @file           : dht11.c
  * @brief          : DHT11 temperature and humidity sensor driver
  ******************************************************************************
  */

#include "dht11.h"
#include "global.h"
#include "FreeRTOS.h"
#include "task.h"

extern volatile uint8_t g_dht_error_count;

/**
  * @brief  Ensure DWT cycle counter is enabled for microsecond delays
  */
static void DHT11_EnableDWT(void)
{
    if (!(CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk)) {
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    }
    if (!(DWT->CTRL & DWT_CTRL_CYCCNTENA_Msk)) {
        DWT->CYCCNT = 0;
        DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    }
}

/**
  * @brief  Delay execution for a given number of microseconds using DWT->CYCCNT
  * @param  us: Microseconds to delay
  */
static void DHT11_DelayUs(uint32_t us)
{
    DHT11_EnableDWT();
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = us * (SystemCoreClock / 1000000UL);
    while ((DWT->CYCCNT - start) < ticks) {
        /* Spin-wait */
    }
}

/**
  * @brief  Configure PA0 as open-drain output with no pull-up/pull-down
  */
static void DHT11_SetPinOutput(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitStruct.Pin = DHT11_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStruct);
}

/**
  * @brief  Configure PA0 as floating input
  */
static void DHT11_SetPinInput(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitStruct.Pin = DHT11_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStruct);
}

/**
  * @brief  Wait for pin to leave a given logic state
  * @param  state: Pin state to wait out (RESET or SET)
  * @param  timeout_us: Maximum time in microseconds to wait
  * @retval 1 on success (pin left state), 0 on timeout
  */
static uint8_t DHT11_WaitState(GPIO_PinState state, uint32_t timeout_us)
{
    DHT11_EnableDWT();
    uint32_t start = DWT->CYCCNT;
    uint32_t timeout_ticks = timeout_us * (SystemCoreClock / 1000000UL);

    while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == state) {
        if ((DWT->CYCCNT - start) >= timeout_ticks) {
            return 0; /* Timeout */
        }
    }
    return 1; /* Pin left given state */
}

/**
  * @brief  Read temperature and humidity from DHT11 sensor
  * @param  temp: Pointer to float where temperature (deg C) will be stored
  * @param  humidity: Pointer to float where relative humidity (%) will be stored
  * @retval 1 on success, 0 on failure/timeout
  */
uint8_t DHT11_ReadData(float *temp, float *humidity)
{
    uint8_t data[5] = {0};

    if (temp == NULL || humidity == NULL) {
        return 0;
    }

    /* 1. Set pin output, pull LOW */
    DHT11_SetPinOutput();
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_RESET);

    /* 2. Start signal: hold LOW for 18ms, yields CPU */
    vTaskDelay(pdMS_TO_TICKS(18));

    /* 3. Set pin HIGH, delay 30us */
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_SET);
    DHT11_DelayUs(30);

    /* 4. Set pin input */
    DHT11_SetPinInput();

    /* 5. Enter critical section for time-sensitive bit capture */
    taskENTER_CRITICAL();

    /* 6. Check response: wait for LOW (line drops), then wait for 80us LOW to end, then 80us HIGH to end */
    if (!DHT11_WaitState(GPIO_PIN_SET, 100)) {
        taskEXIT_CRITICAL();
        g_dht_error_count++;
        return 0;
    }
    if (!DHT11_WaitState(GPIO_PIN_RESET, 100)) {
        taskEXIT_CRITICAL();
        g_dht_error_count++;
        return 0;
    }
    if (!DHT11_WaitState(GPIO_PIN_SET, 100)) {
        taskEXIT_CRITICAL();
        g_dht_error_count++;
        return 0;
    }

    /* 7. Read 40 bits */
    for (int i = 0; i < 40; i++) {
        /* Wait for bit start: LOW-to-HIGH transition (pin leaves LOW) */
        if (!DHT11_WaitState(GPIO_PIN_RESET, 100)) {
            taskEXIT_CRITICAL();
            g_dht_error_count++;
            return 0;
        }

        /* Delay 40us to sample bit value */
        DHT11_DelayUs(40);

        /* Read pin: HIGH -> bit is 1, LOW -> bit is 0 */
        if (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_SET) {
            data[i / 8] |= (uint8_t)(1 << (7 - (i % 8)));

            /* Wait for pin to leave HIGH (return to LOW for next bit) */
            if (!DHT11_WaitState(GPIO_PIN_SET, 100)) {
                taskEXIT_CRITICAL();
                g_dht_error_count++;
                return 0;
            }
        }
    }

    /* 8. Exit critical section */
    taskEXIT_CRITICAL();

    /* 9. Verify checksum */
    uint8_t checksum = (uint8_t)(data[0] + data[1] + data[2] + data[3]);
    if (data[4] == checksum) {
        /* 10. Valid data */
        *humidity = (float)data[0] + (float)data[1] * 0.1f;
        *temp     = (float)data[2] + (float)data[3] * 0.1f;
        g_dht_error_count = 0;
        return 1;
    } else {
        /* 11. Invalid checksum */
        g_dht_error_count++;
        return 0;
    }
}
