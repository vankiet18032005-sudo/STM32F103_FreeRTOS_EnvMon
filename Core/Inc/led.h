#ifndef __LED_H
#define __LED_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

void LED_Init(void);
void LED_Green_Toggle(void);
void LED_Red_Toggle(void);
void LED_Red_On(void);
void LED_Red_Off(void);

#ifdef __cplusplus
}
#endif

#endif /* __LED_H */
