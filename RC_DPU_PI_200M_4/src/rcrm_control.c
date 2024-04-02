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


static XGpio gpio_rcv_monitor; // RC��ȣŽ���� ���� ��ȸ �� ����
static XGpio pll_lock_status; // PLL Lock ���� �� ����
static XGpio log_spi, log_value, log_addr; // Log�� �б� ���� ������

RCRM_MODE rcrm_status;

/*
 * RCV Freq CTRL
 * - Filter Path
 * - Gain CTRL(Sys, Gain)
 * - LNA2,3 (Gain Control Block)
 */
void SetRcrmStatFreq(const uint64_t rcrm_freq)
{
	if((rcrm_freq >= FREQ_400MHz) && (rcrm_freq <= FREQ_1300MHz)) {
		rcrm_status.rcrm_filter_path = RCRM_FILTER_PATH01;
	}
	else if((rcrm_freq > FREQ_1300MHz) && (rcrm_freq <= FREQ_2500MHz)) {
		rcrm_status.rcrm_filter_path = RCRM_FILTER_PATH02;
	}
	else if((rcrm_freq > FREQ_2500MHz) && (rcrm_freq <= FREQ_6000MHz)) {
		rcrm_status.rcrm_filter_path = RCRM_FILTER_PATH03;
	}
	else { }
	if((rcrm_freq >= FREQ_400MHz) && (rcrm_freq <= FREQ_500MHz)) {
		rcrm_status.rcrm_bpf_bank = RCRM_BPF01;
		rcrm_status.rcrm_lpf_bank = RCRM_LPF01;
	}
	else if((rcrm_freq > FREQ_500MHz) && (rcrm_freq<= FREQ_720MHz)) {
		rcrm_status.rcrm_bpf_bank = RCRM_BPF02;
		rcrm_status.rcrm_lpf_bank = RCRM_LPF02;
	}
	else if((rcrm_freq > FREQ_720MHz) && (rcrm_freq <= FREQ_940MHz)) {
		rcrm_status.rcrm_bpf_bank = RCRM_BPF03;
		rcrm_status.rcrm_lpf_bank = RCRM_LPF03;
	}
	else if((rcrm_freq > FREQ_940MHz) && (rcrm_freq <= FREQ_1300MHz)) {
		rcrm_status.rcrm_bpf_bank = RCRM_BPF04;
		rcrm_status.rcrm_lpf_bank = RCRM_LPF03;
	}
	else if((rcrm_freq > FREQ_1300MHz) && (rcrm_freq <= FREQ_1700MHz)) {
		rcrm_status.rcrm_bpf_bank = RCRM_BPF05;
		rcrm_status.rcrm_lpf_bank = RCRM_LPF04;
	}
	else if((rcrm_freq > FREQ_1700MHz) && (rcrm_freq <= FREQ_2500MHz)) {
		rcrm_status.rcrm_bpf_bank = RCRM_BPF06;
		rcrm_status.rcrm_lpf_bank = RCRM_LPF04;
	}
	else if((rcrm_freq > FREQ_2500MHz) && (rcrm_freq <= FREQ_3700MHz)) {
		rcrm_status.rcrm_bpf_bank = RCRM_BPF07;
		rcrm_status.rcrm_lpf_bank = RCRM_LPF05;
	}
	else if((rcrm_freq > FREQ_3700MHz) && (rcrm_freq <= FREQ_6000MHz)) {
		rcrm_status.rcrm_bpf_bank = RCRM_BPF08;
		rcrm_status.rcrm_lpf_bank = RCRM_LPF05;
	}
	else { }

	rcrm_status.rcrm_freq_hz = rcrm_freq;

	SetGainAtten(rcrm_freq);
	MakeRcrmSpiCmdModeCtrl();
}

// LNA 2 (Gain Control Block)
void SetRcrmStatAmpScd(const uint8_t rcrm_amp_scd)
{
	rcrm_status.rcrm_amp_mode2 = rcrm_amp_scd;

	MakeRcrmSpiCmdModeCtrl();
}

void SetRcrmStatSysAtt(const uint8_t rcrm_sys_att)
{
	rcrm_status.rcrm_sys_att = rcrm_sys_att;

	MakeRcrmSpiCmdModeCtrl();
}

void SetRcrmStatGainAtt(const uint8_t rcrm_att_gain)
{
	rcrm_status.rcrm_gain_att = rcrm_att_gain;

	MakeRcrmSpiCmdModeCtrl();
}


/*
 * RCRM BD CTRL
 * SPI - 32 bit, Addr X
 */
void MakeRcrmSpiCmdModeCtrl(void)
{
	uint32_t rcrm_cmd_data = 0;

	// BPF, LPF Path
	if(rcrm_status.rcrm_filter_path == RCRM_FILTER_PATH01) {  							// 400~1300MHz
		rcrm_cmd_data = (rcrm_cmd_data & 0xFFCFFF7EU) | ((uint32_t)1U << 20) | ((uint32_t)1U << 7) | ((uint32_t)0U << 0);
	}
	else if(rcrm_status.rcrm_filter_path == RCRM_FILTER_PATH02) { 						// 1300~2500MHz
		rcrm_cmd_data = (rcrm_cmd_data & 0xFFCFFF7EU) | ((uint32_t)1U << 20) | ((uint32_t)0U << 7) | ((uint32_t)1U << 0);
	}
	else if(rcrm_status.rcrm_filter_path == RCRM_FILTER_PATH03) {  						// 2500~6000MHz
		rcrm_cmd_data = (rcrm_cmd_data & 0xFFCFFF7EU) | ((uint32_t)2U << 20) | ((uint32_t)0U << 7) | ((uint32_t)1U << 0);
	}
	else { }

	// BPF Bank
	switch(rcrm_status.rcrm_bpf_bank) {
	case RCRM_BPF01 :
		rcrm_cmd_data = (rcrm_cmd_data & 0xFFFCFFE7U) | ((uint32_t)1U << 16) | ((uint32_t)0U << 3);
		break;
	case RCRM_BPF02 :
		rcrm_cmd_data = (rcrm_cmd_data & 0xFFFCFFE7U) | ((uint32_t)3U << 16) | ((uint32_t)2U << 3);
		break;
	case RCRM_BPF03 :
		rcrm_cmd_data = (rcrm_cmd_data & 0xFFFCFFE7U) | ((uint32_t)2U << 16) | ((uint32_t)3U << 3);
		break;
	case RCRM_BPF04 :
		rcrm_cmd_data = (rcrm_cmd_data & 0xFFFCFFE7U) | ((uint32_t)0U << 16) | ((uint32_t)1U << 3);
		break;
	case RCRM_BPF05 :
		rcrm_cmd_data = (rcrm_cmd_data & 0xFFFFFF99U) | ((uint32_t)1U << 5) | ((uint32_t)0U << 1);
		break;
	case RCRM_BPF06 :
		rcrm_cmd_data = (rcrm_cmd_data & 0xFFFFFF99U) | ((uint32_t)3U << 5) | ((uint32_t)2U << 1);
		break;
	case RCRM_BPF07 :
		rcrm_cmd_data = (rcrm_cmd_data & 0xFFFFFF99U) | ((uint32_t)2U << 5) | ((uint32_t)3U << 1);
		break;
	case RCRM_BPF08 :
		rcrm_cmd_data = (rcrm_cmd_data & 0xFFFFFF99U) | ((uint32_t)0U << 5) | ((uint32_t)1U << 1);
		break;
	default :
		break;
	}

	// LPF Bank
	switch(rcrm_status.rcrm_lpf_bank) {
	case RCRM_LPF01 :
		rcrm_cmd_data = (rcrm_cmd_data & 0xFF33FFFFU) | ((uint32_t)0U << 22) | ((uint32_t)1U << 18);
		break;
	case RCRM_LPF02 :
		rcrm_cmd_data = (rcrm_cmd_data & 0xFF33FFFFU) | ((uint32_t)3U << 22) | ((uint32_t)2U << 18);
		break;
	case RCRM_LPF03 :
		rcrm_cmd_data = (rcrm_cmd_data & 0xFF33FFFFU) | ((uint32_t)1U << 22) | ((uint32_t)0U << 18);
		break;
	case RCRM_LPF04 :
		rcrm_cmd_data = (rcrm_cmd_data & 0xFF33FFFFU) | ((uint32_t)2U << 22) | ((uint32_t)3U << 18);
		break;
	case RCRM_LPF05 :
		rcrm_cmd_data = (rcrm_cmd_data & 0xFFCFFFFFU) | ((uint32_t)2U << 20);
		break;
	default :
		break;
	}

	// AMP mode2(LNA2)
	if(rcrm_status.rcrm_amp_mode2 == RCRM_BYPASS02) {
		rcrm_cmd_data = (rcrm_cmd_data & 0xFFFFFCFFU) | ((uint32_t)1U << 8);
	}
	else if(rcrm_status.rcrm_amp_mode2 == RCRM_LNA02) {
		rcrm_cmd_data = (rcrm_cmd_data & 0xFFFFFCFFU) | ((uint32_t)2U << 8);
	}
	else { }

	// System Atten
	rcrm_cmd_data = (rcrm_cmd_data & 0xFFFF03FFU) | ((uint32_t)(rcrm_status.rcrm_sys_att) << 10);
	// Gain Atten
	rcrm_cmd_data = (rcrm_cmd_data & 0x03FFFFFFU) | ((uint32_t)(rcrm_status.rcrm_gain_att) << 26);

	SPI_WriteReg(RF_CTRL, 0U, rcrm_cmd_data, 4U);		//32bit SPI control, Addr X
}

