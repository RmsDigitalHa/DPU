/*
 * rcrm_control.h
 *
 *  Created on: 2023. 8. 14.
 *      Author: hwhwh
 */

#ifndef INC_RCRM_CONTROL_H_
#define INC_RCRM_CONTROL_H_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "xparameters.h"
//#include "xspips.h"
#include "xgpio.h"
//#include "xuartlite.h"


// define
#define RCRM_FILTER_PATH01	1U // BPF 1~4, LPF 1~4
#define RCRM_FILTER_PATH02	2U // BPF 5~8, LPF 1~4
#define RCRM_FILTER_PATH03	3U // BPF 5~8, LPF 5

#define RCRM_BPF01	1U
#define RCRM_BPF02	2U
#define RCRM_BPF03	3U
#define RCRM_BPF04	4U
#define RCRM_BPF05	5U
#define RCRM_BPF06	6U
#define RCRM_BPF07	7U
#define RCRM_BPF08	8U

#define RCRM_BYPASS02 	0U
#define RCRM_LNA02		1U

#define RCRM_LPF01	1U
#define RCRM_LPF02	2U
#define RCRM_LPF03	3U
#define RCRM_LPF04	4U
#define RCRM_LPF05	5U

#define FREQ_400MHz		400000000U
#define FREQ_450MHz		450000000U
#define FREQ_500MHz		500000000U
#define FREQ_550MHz		550000000U
#define FREQ_600MHz		600000000U
#define FREQ_610MHz		610000000U
#define FREQ_700MHz		700000000U
#define FREQ_720MHz		720000000U
#define FREQ_800MHz		800000000U
#define FREQ_830MHz		830000000U
#define FREQ_900MHz		900000000U
#define FREQ_940MHz		940000000U
#define FREQ_1000MHz	1000000000U
#define FREQ_1120MHz	1120000000U
#define FREQ_1300MHz	1300000000U
#define FREQ_1500MHz	1500000000U
#define FREQ_1700MHz	1700000000U
#define FREQ_2000MHz	2000000000U
#define FREQ_2100MHz	2100000000U
#define FREQ_2400MHz	2400000000U
#define FREQ_2450MHz	2450000000U
#define FREQ_2500MHz	2500000000U
#define FREQ_3000MHz	3000000000U
#define FREQ_3500MHz	3500000000U
#define FREQ_3600MHz	3600000000U
#define FREQ_3700MHz	3700000000U
#define FREQ_3800MHz	3800000000U
#define FREQ_4000MHz	4000000000U
#define FREQ_4400MHz	4400000000U
#define FREQ_4500MHz	4500000000U
#define FREQ_4850MHz	4850000000U
#define FREQ_5000MHz	5000000000U
#define FREQ_5500MHz	5500000000U
#define FREQ_5700MHz	5700000000U
#define FREQ_5750MHz	5750000000U
#define FREQ_5800MHz	5800000000U
#define FREQ_5850MHz	5850000000U
#define FREQ_5900MHz	5900000000U
#define FREQ_6000MHz	6000100000U

//CH Frequency List
#define FREQ_433MHz		433000000U
#define FREQ_873MHz		873000000U
#define FREQ_923MHz		923000000U
#define FREQ_2425MHz	2425000000U
#define FREQ_2475MHz	2475000000U
#define FREQ_5035MHz	5035000000U
#define FREQ_5085MHz	5085000000U
#define FREQ_5725MHz	5725000000U
#define FREQ_5775MHz	5775000000U
#define FREQ_5825MHz	5825000000U
#define FREQ_5875MHz	5875000000U



//// GPIO ID 정의
//#define GPIO_RCV_MONITOR_DEV_ID  XPAR_STATUS_MONITOR_DEVICE_ID
//#define GPIO_PLL_LOCK_DEV_ID 	 XPAR_CAL_MISO_DEVICE_ID

// 사용자 정의 데이터 형
typedef struct rcrm_mode_state{
	uint8_t rcrm_amp_mode2;		// LNA2
	uint8_t rcrm_filter_path;
	uint8_t rcrm_bpf_bank;
	uint8_t rcrm_lpf_bank;
	uint8_t rcrm_sys_att;
	uint8_t rcrm_gain_att;
	uint64_t rcrm_freq_hz;		// 수신기(Filter 경로 설정) 주파수
	uint64_t rmrm_bit_freq;		// Bit 주파수
}RCRM_MODE;


extern RCRM_MODE rcrm_status;

// 외부 함수 정의
void SetRcrmStatFreq(const uint64_t rcrm_freq);
void SetRcrmStatAmpScd(const uint8_t rcrm_amp_scd);
void SetRcrmStatSysAtt(const uint8_t rcrm_sys_att);
void SetRcrmStatGainAtt(const uint8_t rcrm_att_gain);
void MakeRcrmSpiCmdModeCtrl(void);


#endif /* INC_RCRM_CONTROL_H_ */
