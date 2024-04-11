/*
 * user_func.c
 *
 *  Created on: 2023. 9. 25.
 *      Author: hadud
 */

// stdlibs
#include <error.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// platform drivers
#include "xstatus.h"
#include "xil_io.h"
#include "delay.h"
#include "util.h"
#include "sleep.h"
#include "netif/xadapter.h"

// talise
#include "talise.h"
#include "talise_jesd204.h"
#include "talise_arm.h"
#include "talise_radioctrl.h"
#include "talise_cals.h"
#include "talise_error.h"
#include "talise_cals_types.h"
#include "talise_rx.h"

// user
#include "udp_server.h"
#include "user_func.h"
#include "icd_parser.h"
#include "dma.h"
#include "axi_rc_spectrum.h"
#include "adrv9008.h"
#include "rts_spectrum_ctrl.h"
#include "rcrm_control.h"


static uint64_t old_freq = 0;
extern uint32_t dpu_iter_count;
extern uint32_t dpu_ref_level;
extern uint32_t dpu_win_func;
extern uint8_t SPEC_BUF_PREV[FFT_2048_BIN + ICD_HEADER_SIZE + SPEC_HEADER_SIZE];
extern uint8_t SPEC_BUF_CUR[FFT_2048_BIN + ICD_HEADER_SIZE + SPEC_HEADER_SIZE];

extern uint32_t *AddrSpecCurHeader;

static uint32_t *AddrSpecBuffer;
static uint32_t *AddrSpecBufTMP;
static uint32_t *AddrSpecBufferHeader;



static uint64_t FreqList[13] = {433000000U, 873000000U, 923000000U, 2425000000U, 2475000000U, 5035000000U,
						5085000000U, 5725000000U, 5775000000U, 5825000000U, 5875000000U, 433000000U, 873000000U};


int ChangeLoFreq(taliseDevice_t * const pd, uint64_t freq)
{
	uint32_t talAction = TALACT_NO_ACTION;
	uint8_t pllLockStatus = 0;

	if(freq >= 6000000000U){
		freq = (uint64_t)5999985000;
	}

	talAction = TALISE_radioOff(pd);
	if (talAction != TALACT_NO_ACTION) {
		/*** < User: decide what to do based on Talise recovery action returned > ***/
		printf("error: TALISE_radioOff() failed\n");
		talAction = TALACT_NO_ACTION;
	}

	talAction = TALISE_setRfPllFrequency(pd, TAL_RF_PLL, freq);
	if (talAction != TALACT_NO_ACTION) {
		printf("error: TALISE_setRfPllFrequency() failed\n");
		return 1;
	}

	/*** < wait 200ms for PLLs to lock - user code here > ***/
	mdelay(200);

	talAction = TALISE_getPllsLockStatus(pd, &pllLockStatus);
	if ((pllLockStatus & 0x07U) != 0x07U) {
		/*< user code - ensure lock of all PLLs before proceeding>*/
		printf("error: RFPLL not locked\n");
	}

	talAction = TALISE_radioOn(pd);
	if (talAction != TALACT_NO_ACTION) {
		printf("error: TALISE_radioOn() failed\n");
		return 1;
	}

	return 0;
}


int HoppingStart(taliseDevice_t * const pd, const uint64_t freq){
	uint32_t talAction = TALACT_NO_ACTION;

	hopp_config.fhmGpioPin = TAL_GPIO_INVALID;
	hopp_config.fhmMinFreq_MHz = 170;
	hopp_config.fhmMaxFreq_MHz = 5900;

	hopp_mode.fhmInitFrequency_Hz = freq;
	hopp_mode.fhmEnable = 1;
	hopp_mode.enableMcsSync = 0;
	hopp_mode.fhmTriggerMode = TAL_FHM_NON_GPIO_MODE;

	talAction = TALISE_radioOff(pd);
	if (talAction != TALACT_NO_ACTION) {
		/*** < User: decide what to do based on Talise recovery action returned > ***/
		printf("error: TALISE_radioOff() failed\n");
		talAction = TALACT_NO_ACTION;
	}

	usleep(DELAY_HOP);

	//After radio off state	ug-1295 121p
	talAction = TALISE_setFhmConfig(pd, &hopp_config);
	if (talAction != TALACT_NO_ACTION) {
		printf("error: TALISE_setFhmConfig() failed\n");
		return 1;
	}

	talAction = TALISE_radioOn(pd);
	if (talAction != TALACT_NO_ACTION) {
		printf("error: TALISE_radioOn() failed\n");
		return 1;
	}

	talAction = TALISE_setFhmMode(pd, &hopp_mode);
	if (talAction != TALACT_NO_ACTION) {
		printf("error: TALISE_setFhmMode() failed\n");
		return 1;
	}
	return 0;
}


static int HoppingNext(taliseDevice_t * const pd, const uint64_t next_freq){
	uint32_t talAction = TALACT_NO_ACTION;

	talAction = TALISE_setFhmHop(pd, next_freq);
	if (talAction != TALACT_NO_ACTION) {
		printf("error: TALISE_setFhmHop() failed\n");
		return 1;
	}

	return 0;
}

int HoppingEnd(taliseDevice_t * const pd, const uint64_t freq){
	uint32_t talAction = TALACT_NO_ACTION;

	hopp_mode.fhmInitFrequency_Hz = freq;
	hopp_mode.fhmEnable = 0;
	hopp_mode.fhmTriggerMode = TAL_FHM_NON_GPIO_MODE;
	hopp_mode.fhmExitMode = TAL_FHM_QUICK_EXIT;

	talAction = TALISE_setFhmMode(pd, &hopp_mode);
	if (talAction != TALACT_NO_ACTION) {
		printf("error: TALISE_setFhmMode() failed\n");
		return 1;
	}
	return 0;
}


int CHScanStart(const uint8_t CH, const uint8_t ITER_CNT){
	uint64_t center_freq = 0;
	int Status = 0;
	uint8_t FrameDone = 0;
	uint8_t Done_CNT = 0;

	tal.devHalInfo = (void *) &hal;

	for(uint8_t i = 2U; i < (CH + 2U); i++){
		(void)memset((uint8_t *)&SPEC_BUF_PREV, 0x00, sizeof(SPEC_BUF_PREV));
		(void)memset((uint8_t *)&SPEC_BUF_CUR, 0x00, sizeof(SPEC_BUF_CUR));

		center_freq = FreqList[i] + FREQ_OFFSET - FREQ_NCO;
		DPU_STATUS.CenterFreq = FreqList[i - 1U];		//수정(240311)
		SetRcrmStatFreq(FreqList[i - 1U] + FREQ_OFFSET);

		Status = AdrvGainCtrl((uint64_t)(FreqList[i - 1U] + FREQ_OFFSET));
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Status = HoppingNext(&tal, center_freq);
		if (Status != XST_SUCCESS) {
			printf("ADRV Freq hopping fail\r\n");
			return XST_FAILURE;
		}
		usleep(100);
		(void)rts_start(RC_SPCTRUM_BaseAddr, dpu_iter_count, dpu_ref_level, dpu_win_func);

		while(true){
			FrameDone = RTS_SPECTRUM_CTRL_mReadReg(RC_SPCTRUM_BaseAddr, REG_RTS_FRAME_DONE);
			if(FrameDone == 1U){
				Done_CNT += 1U;
				(void)RxDmaData();			//수정(240311)
			}
			if(Done_CNT == (ITER_CNT + 2U)){
				Done_CNT = 0;
				break;
			}
			else if(Done_CNT > 1U){		//1Cycle은 기존 BRAM 초기화 필요함(x)
				(void)RxDmaData();
				Status = IterSpectrum();
				if (Status != XST_SUCCESS) {
					printf("ADRV Freq hopping fail\r\n");
					return XST_FAILURE;
				}
				Done_CNT += 1U;
			}
			else { }

		}
		(void)rts_end(RC_SPCTRUM_BaseAddr);
		(void)TransferData();
		xemacif_input(&server_netif);
	}

	return 0;
}

int BWScanStart(const uint64_t FREQ, uint64_t BW, uint32_t RBW){
	uint64_t center_freq = 0;
	uint8_t FrameDone = 0;
	uint8_t Done_CNT = 0;
	int Status = 0;

	tal.devHalInfo = (void *) &hal;

	center_freq = FREQ + FREQ_OFFSET - FREQ_NCO;

	(void)rts_end(RC_SPCTRUM_BaseAddr);


	if(old_freq != center_freq){
		Status = AdrvGainCtrl((uint64_t)(center_freq + FREQ_NCO));
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Status = ChangeLoFreq(&tal, center_freq);

		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	}

	(void)rts_start(RC_SPCTRUM_BaseAddr, dpu_iter_count, dpu_ref_level, dpu_win_func);

	while(true){
		FrameDone = RTS_SPECTRUM_CTRL_mReadReg(RC_SPCTRUM_BaseAddr, REG_RTS_FRAME_DONE);
		if(FrameDone == 1U){
			(void)RxDmaData();
			break;
		}

	}

	old_freq = center_freq;

	(void)TransferData();

	xemacif_input(&server_netif);

	return XST_SUCCESS;
}

static int IterSpectrum(void){
	uint16_t spec_length = (uint16_t)(spec_packet_size - ICD_HEADER_SIZE - SPEC_HEADER_SIZE) / (uint16_t)2;
	uint16_t *AddrPrev = (uint16_t *)&SPEC_BUF_PREV[ICD_HEADER_SIZE + SPEC_HEADER_SIZE];
	uint16_t *AddrCUR = (uint16_t *)&SPEC_BUF_CUR[ICD_HEADER_SIZE + SPEC_HEADER_SIZE];

	for(uint16_t spec_cnt = 0; spec_cnt < spec_length; spec_cnt++){
		if((int16_t)(AddrCUR[spec_cnt]) > (int16_t)(AddrPrev[spec_cnt])){

		}
		else{

			AddrCUR[spec_cnt] = AddrPrev[spec_cnt];
		}
	}

	return XST_SUCCESS;
}

static int AdrvGainCtrl(const uint64_t FREQ){
	int Status = 0;

	//2?? ??????, EMI, ???경시??? 진행
	if((FREQ >= FREQ_400MHz) && (FREQ < FREQ_500MHz)){
		Status = SetAdrvGain(&tal, 253U);
	}
	else if((FREQ >= FREQ_500MHz) && (FREQ < FREQ_1000MHz)){
		Status = SetAdrvGain(&tal, 255U);
	}
	else if((FREQ >= FREQ_1000MHz) && (FREQ < FREQ_1500MHz)){
		Status = SetAdrvGain(&tal, 253U);
	}
	else if((FREQ >= FREQ_1500MHz) && (FREQ < FREQ_2000MHz)){
		Status = SetAdrvGain(&tal, 251U);
	}
	else if((FREQ >= FREQ_2000MHz) && (FREQ < FREQ_2500MHz)){
		Status = SetAdrvGain(&tal, 254U);
	}
	else if((FREQ >= FREQ_2500MHz) && (FREQ < FREQ_3700MHz)){
		Status = SetAdrvGain(&tal, 251U);
	}
	else if((FREQ >= FREQ_3700MHz) && (FREQ < FREQ_4400MHz)){
		Status = SetAdrvGain(&tal, 250U);
	}
	else if((FREQ >= FREQ_4400MHz) && (FREQ < FREQ_5700MHz)){
		Status = SetAdrvGain(&tal, 251U);
	}
	else if((FREQ >= FREQ_5700MHz) && (FREQ < FREQ_5800MHz)){
		Status = SetAdrvGain(&tal, 250U);
	}
	else if((FREQ >= FREQ_5800MHz) && (FREQ < FREQ_5850MHz)){
		Status = SetAdrvGain(&tal, 248U);
	}
	else if((FREQ >= FREQ_5850MHz) && (FREQ < FREQ_5900MHz)){
		Status = SetAdrvGain(&tal, 247U);
	}
	else if((FREQ >= FREQ_5900MHz) && (FREQ <= FREQ_6000MHz)){
		Status = SetAdrvGain(&tal, 248U);
	}
	else{
		Status = SetAdrvGain(&tal, 255U);
		printf("AdrvGainCtrl - Frequency is out of range.\r\n");
	}

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	else{
		return XST_SUCCESS;
	}
}


int SetAdrvGain(taliseDevice_t * const pd, const uint8_t gain){
	uint32_t talAction = TALACT_NO_ACTION;

	talAction = TALISE_setRxManualGain(pd, TAL_RX1, gain);
	if (talAction != TALACT_NO_ACTION) {
		printf("error: TALISE_radioOn() failed\n");
		return 1;
	}
	return 0;
}
