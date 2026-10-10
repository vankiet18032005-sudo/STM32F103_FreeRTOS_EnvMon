#ifndef __DHT11_H
#define __DHT11_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

#define DHT11_PORT  GPIOA
#define DHT11_PIN   GPIO_PIN_0

uint8_t DHT11_ReadData(float *temp, float *humidity);

#ifdef __cplusplus
}
#endif

#endif /* __DHT11_H */
