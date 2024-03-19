/*
 * headless.h
 *
 *  Created on: 2023. 8. 1.
 *      Author: hadud
 */

#ifndef SRC_APP_ADRV9008_H_
#define SRC_APP_ADRV9008_H_


#include "adi_hal.h"
#include "app_talise.h"

struct adi_hal hal;
taliseDevice_t tal;

//static uint32_t start = 0;


int Init_ADRV9008(void);


#endif /* SRC_APP_ADRV9008_H_ */
