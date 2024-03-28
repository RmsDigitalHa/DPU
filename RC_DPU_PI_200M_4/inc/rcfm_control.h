/*
 * rcfm_control.h
 *
 *  Created on: 2023. 8. 16.
 *      Author: hwhwh
 */

#ifndef INC_RCFM_CONTROL_H_
#define INC_RCFM_CONTROL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "xparameters.h"
#include "xgpio.h"

// define
#define RCFM_BYPASS 	0U
#define RCFM_LNA		1U

#define RCFM_ANT_PATH	0U
#define RCFM_BIT_PATH	1U

#define RCFM_CAL_DIS	0U
#define RCFM_CAL_EN		1U

#define RCFM_ANT_BIAS_OFF	0U
#define RCFM_ANT_BIAS_ON	1U

#define 	PLL_TABLE_SIZE	16U

#define		MIN_Freq	400000000U
#define		MAX_Freq	6000000000U
#define		LAST_Freq	3550000000U


typedef struct rcfm_mode_state{
	uint8_t rcfm_amp_mode1;		// 'MODE_CTRL_V1', LNA1 CTRL, GPIO_2
	uint8_t rcfm_amp_mode2;		// 'MODE_CTRL_V2', LNA1 CTRL, GPIO_3
	uint8_t rcfm_ant_bias;		// 'BIAS_CTRL', ANT BIAS PWR ON/OFF, GPIO_4
	uint8_t rcfm_rf_select;		// 'BIT_V1', ANT/BIT PATH, GPIO_5
	uint8_t rcfm_cal_en;		// 'LO_CTRL(CAL_EN)', BIT Enable, GPIO_6
	uint64_t rcfm_cal_freq;		// BIT Freq

}RCFM_MODE;

typedef struct
{
	uint64_t		u64StartFreq;
	uint8_t			u8Segment1;
	uint8_t			u8Segment2;
	uint8_t		 	u8Segment3;
	uint8_t			u8SegValue1;
	uint8_t			u8SegValue2;
	uint8_t		 	u8SegValue3;
	uint8_t			u8SegEN1;
	uint8_t			u8SegEN2;
	uint8_t		 	u8SegEN3;
	uint8_t		 	u8SegSEL;
}
typTableLMX2582;

typTableLMX2582 TableLMX2582[16];


extern RCFM_MODE rcfm_status;

void SetRcfmStatAmpFst(const uint8_t rcfm_amp_fst);
void SetRcfmStatPath(const uint8_t rcfm_path);
void SetRcfmStatBitEn(const uint8_t rcfm_bit_en);
void SetRcfmStatBitFreq(const uint64_t Freq);
void SetRcfmStatPathANT(const uint8_t rcfm_path_lna);
void Init_BIT_PLL(void);
static typTableLMX2582 GetPLLValue(const uint64_t TargetFreq);

#endif /* INC_RCFM_CONTROL_H_ */
