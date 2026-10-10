#ifndef __KEY_H
#define __KEY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

typedef enum {
    KEY_NONE = 0,
    KEY1_PRESSED,
    KEY2_PRESSED
} KeyEvent_t;

void KEY_Scan(void);

#ifdef __cplusplus
}
#endif

#endif /* __KEY_H */
