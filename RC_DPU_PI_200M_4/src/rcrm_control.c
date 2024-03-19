/*
 * rcrm_control.c
 *
 *  Created on: 2023. 8. 14.
 *      Author: hwhwh
 *
 */


#include <math.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "xparameters.h"
#include "xgpio.h"
#include "xgpiops.h"
#include "sleep.h"
#include "rcrm_control.h"
#include "Init.h"


XGpio gpio_rcv_monitor; // RC신호탐지기 상태 조회 값 변수
XGpio pll_lock_status; // PLL Lock 상태 값 변수
XGpio log_spi, log_value, log_addr; // Log값 읽기 위한 변수들

RCRM_MODE rcrm_status;

/*
 * RCV Freq CTRL
 * - Filter Path
 * - Gain CTRL(Sys, Gain)
 * - LNA2,3 (Gain Control Block)
 */
void SetRcrmStatFreq(uint64_t rcrm_freq)
{
	if(rcrm_freq >= FREQ_400MHz && rcrm_freq <= FREQ_1300MHz)
		rcrm_status.rcrm_filter_path = RCRM_FILTER_PATH01;
	else if(rcrm_freq > FREQ_1300MHz && rcrm_freq <= FREQ_2500MHz)
		rcrm_status.rcrm_filter_path = RCRM_FILTER_PATH02;
	else if(rcrm_freq > FREQ_2500MHz && rcrm_freq <= FREQ_6000MHz)
		rcrm_status.rcrm_filter_path = RCRM_FILTER_PATH03;

	if(rcrm_freq >= FREQ_400MHz && rcrm_freq <= FREQ_500MHz) {
		rcrm_status.rcrm_bpf_bank = RCRM_BPF01;
		rcrm_status.rcrm_lpf_bank = RCRM_LPF01;
	}
	else if(rcrm_freq > FREQ_500MHz && rcrm_freq<= FREQ_720MHz) {
		rcrm_status.rcrm_bpf_bank = RCRM_BPF02;
		rcrm_status.rcrm_lpf_bank = RCRM_LPF02;
	}
	else if(rcrm_freq > FREQ_720MHz && rcrm_freq <= FREQ_940MHz) {
		rcrm_status.rcrm_bpf_bank = RCRM_BPF03;
		rcrm_status.rcrm_lpf_bank = RCRM_LPF03;
	}
	else if(rcrm_freq > FREQ_940MHz && rcrm_freq <= FREQ_1300MHz) {
		rcrm_status.rcrm_bpf_bank = RCRM_BPF04;
		rcrm_status.rcrm_lpf_bank = RCRM_LPF03;
	}
	else if(rcrm_freq > FREQ_1300MHz && rcrm_freq <= FREQ_1700MHz) {
		rcrm_status.rcrm_bpf_bank = RCRM_BPF05;
		rcrm_status.rcrm_lpf_bank = RCRM_LPF04;
	}
	else if(rcrm_freq > FREQ_1700MHz && rcrm_freq <= FREQ_2500MHz) {
		rcrm_status.rcrm_bpf_bank = RCRM_BPF06;
		rcrm_status.rcrm_lpf_bank = RCRM_LPF04;
	}
	else if(rcrm_freq > FREQ_2500MHz && rcrm_freq <= FREQ_3700MHz) {
		rcrm_status.rcrm_bpf_bank = RCRM_BPF07;
		rcrm_status.rcrm_lpf_bank = RCRM_LPF05;
	}
	else if(rcrm_freq > FREQ_3700MHz && rcrm_freq <= FREQ_6000MHz) {
		rcrm_status.rcrm_bpf_bank = RCRM_BPF08;
		rcrm_status.rcrm_lpf_bank = RCRM_LPF05;
	}

	rcrm_status.rcrm_freq_hz = rcrm_freq;

	SetGainAtten(rcrm_freq);
	MakeRcrmSpiCmdModeCtrl();
}


void SetRcrmStatBpf(uint8_t rcrm_bpf)
{
	rcrm_status.rcrm_bpf_bank = rcrm_bpf;

	MakeRcrmSpiCmdModeCtrl();
}

void SetRcrmStatLpf(uint8_t rcrm_lpf)
{
	rcrm_status.rcrm_lpf_bank = rcrm_lpf;

	MakeRcrmSpiCmdModeCtrl();
}

// LNA 2 (Gain Control Block)
void SetRcrmStatAmpScd(uint8_t rcrm_amp_scd)
{
	rcrm_status.rcrm_amp_mode2 = rcrm_amp_scd;

	MakeRcrmSpiCmdModeCtrl();
}

void SetRcrmStatSysAtt(uint8_t rcrm_sys_att)
{
	rcrm_status.rcrm_sys_att = rcrm_sys_att;

	MakeRcrmSpiCmdModeCtrl();
}

void SetRcrmStatGainAtt(uint8_t rcrm_att_gain)
{
	rcrm_status.rcrm_gain_att = rcrm_att_gain;

	MakeRcrmSpiCmdModeCtrl();
}


/*
 * RCRM BD CTRL
 * SPI - 32 bit, Addr X
 */
void MakeRcrmSpiCmdModeCtrl()
{
	uint32_t rcrm_cmd_data = 0;

	// BPF, LPF Path
	if(rcrm_status.rcrm_filter_path == RCRM_FILTER_PATH01)  							// 400~1300MHz
		rcrm_cmd_data = (rcrm_cmd_data & 0xFFCFFF7E) | (1 << 20) | (1 << 7) | (0 << 0);
	else if(rcrm_status.rcrm_filter_path == RCRM_FILTER_PATH02)  						// 1300~2500MHz
		rcrm_cmd_data = (rcrm_cmd_data & 0xFFCFFF7E) | (1 << 20) | (0 << 7) | (1 << 0);
	else if(rcrm_status.rcrm_filter_path == RCRM_FILTER_PATH03)  						// 2500~6000MHz
		rcrm_cmd_data = (rcrm_cmd_data & 0xFFCFFF7E) | (2 << 20) | (0 << 7) | (1 << 0);

	// BPF Bank
	switch(rcrm_status.rcrm_bpf_bank) {
	case RCRM_BPF01 :
		rcrm_cmd_data = (rcrm_cmd_data & 0xFFFCFFE7) | (1 << 16) | (0 << 3);
		break;
	case RCRM_BPF02 :
		rcrm_cmd_data = (rcrm_cmd_data & 0xFFFCFFE7) | (3 << 16) | (2 << 3);
		break;
	case RCRM_BPF03 :
		rcrm_cmd_data = (rcrm_cmd_data & 0xFFFCFFE7) | (2 << 16) | (3 << 3);
		break;
	case RCRM_BPF04 :
		rcrm_cmd_data = (rcrm_cmd_data & 0xFFFCFFE7) | (0 << 16) | (1 << 3);
		break;
	case RCRM_BPF05 :
		rcrm_cmd_data = (rcrm_cmd_data & 0xFFFFFF99) | (1 << 5) | (0 << 1);
		break;
	case RCRM_BPF06 :
		rcrm_cmd_data = (rcrm_cmd_data & 0xFFFFFF99) | (3 << 5) | (2 << 1);
		break;
	case RCRM_BPF07 :
		rcrm_cmd_data = (rcrm_cmd_data & 0xFFFFFF99) | (2 << 5) | (3 << 1);
		break;
	case RCRM_BPF08 :
		rcrm_cmd_data = (rcrm_cmd_data & 0xFFFFFF99) | (0 << 5) | (1 << 1);
		break;
	}

	// LPF Bank
	switch(rcrm_status.rcrm_lpf_bank) {
	case RCRM_LPF01 :
		rcrm_cmd_data = (rcrm_cmd_data & 0xFF33FFFF) | (0 << 22) | (1 << 18);
		break;
	case RCRM_LPF02 :
		rcrm_cmd_data = (rcrm_cmd_data & 0xFF33FFFF) | (3 << 22) | (2 << 18);
		break;
	case RCRM_LPF03 :
		rcrm_cmd_data = (rcrm_cmd_data & 0xFF33FFFF) | (1 << 22) | (0 << 18);
		break;
	case RCRM_LPF04 :
		rcrm_cmd_data = (rcrm_cmd_data & 0xFF33FFFF) | (2 << 22) | (3 << 18);
		break;
	case RCRM_LPF05 :
		rcrm_cmd_data = (rcrm_cmd_data & 0xFFCFFFFF) | (2 << 20);
		break;
	}

	// AMP mode2(LNA2)
	if(rcrm_status.rcrm_amp_mode2 == RCRM_BYPASS02)
		rcrm_cmd_data = (rcrm_cmd_data & 0xFFFFFCFF) | (1 << 8);
	else if(rcrm_status.rcrm_amp_mode2 == RCRM_LNA02)
		rcrm_cmd_data = (rcrm_cmd_data & 0xFFFFFCFF) | (2 << 8);

	// System Atten
	rcrm_cmd_data = (rcrm_cmd_data & 0xFFFF03FF) | (rcrm_status.rcrm_sys_att << 10);
	// Gain Atten
	rcrm_cmd_data = (rcrm_cmd_data & 0x03FFFFFF) | (rcrm_status.rcrm_gain_att << 26);

	SPI_WriteReg(RF_CTRL, NULL, rcrm_cmd_data, 4);		//32bit SPI control, Addr X
}

