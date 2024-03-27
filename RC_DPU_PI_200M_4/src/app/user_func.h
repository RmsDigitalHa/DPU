/*
 * user_func.h
 *
 *  Created on: 2023. 9. 25.
 *      Author: hadud
 */

#ifndef SRC_APP_USER_FUNC_H_
#define SRC_APP_USER_FUNC_H_

#include "adrv9008.h"
#include "talise.h"
#include "talise_jesd204.h"
#include "talise_arm.h"
#include "talise_radioctrl.h"
#include "talise_cals.h"
#include "talise_error.h"
#include "talise_cals_types.h"

taliseFhmConfig_t	hopp_config;
taliseFhmMode_t		hopp_mode;
static taliseFhmStatus_t	hopp_status;

//#define BW_2DECI	50000000	//50MHz
//#define BW_2DECI	48000000	//50MHz
//#define START_FREQ	400000000

#define FREQ_OFFSET	30000U
//#define BW_SPEC_OFFSET	120000
#define FREQ_NCO	62640000U	//64.11M(origin) -> 1.2M Shift

#define DELAY_HOP	2000



int ChangeLoFreq(taliseDevice_t * const pd, uint64_t freq);
int SetAdrvGain(taliseDevice_t * const pd, uint8_t gain);
int HoppingStart(taliseDevice_t * const pd, uint64_t freq);
static int HoppingNext(taliseDevice_t * const pd, uint64_t next_freq);
int HoppingEnd(taliseDevice_t * const pd, uint64_t freq);
int CHScanStart(uint8_t CH, uint8_t ITER_CNT);
static void IterSpectrum(void);
static int AdrvGainCtrl(uint64_t FREQ);
int BWScanStart(uint64_t FREQ, uint64_t BW, uint16_t RBW);


#endif /* SRC_APP_USER_FUNC_H_ */
