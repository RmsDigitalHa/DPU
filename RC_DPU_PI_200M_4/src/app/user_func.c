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


uint64_t old_freq = 0;
extern uint32_t dpu_iter_count;
extern uint32_t dpu_ref_level;
extern uint32_t dpu_win_func;
extern uint8_t SPEC_BUF_PREV[FFT_2048_BIN + ICD_HEADER_SIZE + SPEC_HEADER_SIZE];
extern uint8_t SPEC_BUF_CUR[FFT_2048_BIN + ICD_HEADER_SIZE + SPEC_HEADER_SIZE];

extern uint32_t *AddrSpecHeader;
extern uint32_t *AddrSpecPrevHeader;
extern uint32_t *AddrSpecCurHeader;

extern RECV_SETTING DPU_STATUS;
uint32_t *AddrSpecBuffer;
uint32_t *AddrSpecBufTMP;
uint32_t *AddrSpecBufferHeader;



uint64_t FreqList[13] = {433000000U, 873000000U, 923000000U, 2425000000U, 2475000000U, 5035000000U,
						5085000000U, 5725000000U, 5775000000U, 5825000000U, 5875000000U, 433000000U, 873000000U};


int ChangeLoFreq(taliseDevice_t * const pd, uint64_t freq)
{
	uint32_t talAction = TALACT_NO_ACTION;
	uint8_t pllLockStatus = 0;

	if(freq >= 6000000000U){
		freq = 5999985000;
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


int HoppingStart(taliseDevice_t * const pd, uint64_t freq){
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


int HoppingNext(taliseDevice_t * const pd, uint64_t next_freq){
	uint32_t talAction = TALACT_NO_ACTION;

	talAction = TALISE_setFhmHop(pd, next_freq);
	if (talAction != TALACT_NO_ACTION) {
		printf("error: TALISE_setFhmHop() failed\n");
		return 1;
	}

	return 0;
}

int HoppingEnd(taliseDevice_t * const pd, uint64_t freq){
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


int CHScanStart(uint8_t CH, uint8_t ITER_CNT){
	uint64_t center_freq = 0;
	int Status = 0;
	uint8_t FrameDone = 0;
	uint8_t Done_CNT = 0;

	tal.devHalInfo = (void *) &hal;

	for(uint16_t i = 2U; i < CH + 2U; i++){
		memset((uint8_t *)&SPEC_BUF_PREV, 0x00, sizeof(SPEC_BUF_PREV));
		memset((uint8_t *)&SPEC_BUF_CUR, 0x00, sizeof(SPEC_BUF_CUR));

		center_freq = FreqList[i] + FREQ_OFFSET - FREQ_NCO;
//		DPU_STATUS.CenterFreq = FreqList[i - 2] + FREQ_OFFSET - FREQ_NCO;
//		DPU_STATUS.CenterFreq = FreqList[i - 2];
		DPU_STATUS.CenterFreq = FreqList[i - 1];		//����(240311)
//		SetRcrmStatFreq(FreqList[i - 1] + FREQ_OFFSET - FREQ_NCO);
		SetRcrmStatFreq(FreqList[i - 1] + FREQ_OFFSET);
//		SetRcrmStatFreq(FreqList[i] + FREQ_OFFSET);		//����(240311)

//		Status = AdrvGainCtrl((uint64_t)(center_freq + FREQ_NCO));
		Status = AdrvGainCtrl((uint64_t)(FreqList[i - 1] + FREQ_OFFSET));
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Status = HoppingNext(&tal, center_freq);
		if (Status != XST_SUCCESS) {
			printf("ADRV Freq hopping fail\r\n");
			return XST_FAILURE;
		}
		usleep(100);
		rts_start(RC_SPCTRUM_BaseAddr, dpu_iter_count, dpu_ref_level, dpu_win_func);

		while(1){
			FrameDone = RTS_SPECTRUM_CTRL_mReadReg(RC_SPCTRUM_BaseAddr, REG_RTS_FRAME_DONE);
			if((FrameDone == 1U) && (Done_CNT < 2U)){
				Done_CNT += 1U;
				RxDmaData();			//����(240311)
//				break;
			}
			if(Done_CNT == (ITER_CNT + 2U)){
				Done_CNT = 0;
				break;
			}
			else if(Done_CNT > 1U){		//1Cycle�� ���� BRAM �ʱ�ȭ �ʿ���(x)
				RxDmaData();
				IterSpectrum();
				Done_CNT += 1U;
			}
//			if(Done_CNT == 2){
//				RxDmaData();
////				IterSpectrum();
//				Done_CNT = 0;
//				break;
//			}
		}
		rts_end(RC_SPCTRUM_BaseAddr);
		TransferData();
		xemacif_input(&server_netif);
	}

	return 0;
}

int BWScanStart(uint64_t FREQ, uint64_t BW, uint16_t RBW){
	uint64_t center_freq = 0;
	uint8_t FrameDone = 0;
	uint8_t Done_CNT = 0;
	int Status = 0;

	tal.devHalInfo = (void *) &hal;

	center_freq = FREQ + FREQ_OFFSET - FREQ_NCO;

	rts_end(RC_SPCTRUM_BaseAddr);


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

	rts_start(RC_SPCTRUM_BaseAddr, dpu_iter_count, dpu_ref_level, dpu_win_func);

	while(1){
		FrameDone = RTS_SPECTRUM_CTRL_mReadReg(RC_SPCTRUM_BaseAddr, REG_RTS_FRAME_DONE);
		if(FrameDone == 1U){
			RxDmaData();
			break;
//			Done_CNT += 1;
		}
//		if(Done_CNT == 1){
//			RxDmaData();
//			Done_CNT = 0;
//			break;
//		}
	}

	old_freq = center_freq;

	TransferData();

	xemacif_input(&server_netif);

	return XST_SUCCESS;
}

void IterSpectrum(){
	uint16_t spec_length = (spec_packet_size - ICD_HEADER_SIZE - SPEC_HEADER_SIZE) / 2;
	uint16_t *AddrPrev = (uint16_t *)&SPEC_BUF_PREV[ICD_HEADER_SIZE + SPEC_HEADER_SIZE];
	uint16_t *AddrCUR = (uint16_t *)&SPEC_BUF_CUR[ICD_HEADER_SIZE + SPEC_HEADER_SIZE];

	for(int spec_cnt = 0; spec_cnt < spec_length; spec_cnt++){
		if((int16_t)(AddrCUR[spec_cnt]) > (int16_t)(AddrPrev[spec_cnt])){
			// AddrSpecCurHeader[((ICD_HEADER_SIZE + SPEC_HEADER_SIZE)/4) + i] = AddrSpecCurHeader[((ICD_HEADER_SIZE + SPEC_HEADER_SIZE)/4) + i];
		}
		else{
			//memcpy(AddrCUR[spec_cnt], AddrPrev[spec_cnt], sizeof(AddrPrev[spec_cnt]));
			AddrCUR[spec_cnt] = AddrPrev[spec_cnt];
		}
	}
}

int AdrvGainCtrl(uint64_t FREQ){
	int Status = 0;

	// 2,3�� ����
//	if(FREQ >= FREQ_400MHz && FREQ < FREQ_500MHz){
//		Status = SetAdrvGain(&tal, 253);
//	}
//	else if(FREQ >= FREQ_500MHz && FREQ < FREQ_1000MHz){
//		Status = SetAdrvGain(&tal, 255);
//	}
//	else if(FREQ >= FREQ_1000MHz && FREQ < FREQ_1500MHz){
//		Status = SetAdrvGain(&tal, 253);
//	}
//	else if(FREQ >= FREQ_1500MHz && FREQ < FREQ_2000MHz){
//		Status = SetAdrvGain(&tal, 251);
//	}
//	else if(FREQ >= FREQ_2000MHz && FREQ < FREQ_2500MHz){
//		Status = SetAdrvGain(&tal, 254);
//	}
//	else if(FREQ >= FREQ_2500MHz && FREQ < FREQ_3700MHz){
//		Status = SetAdrvGain(&tal, 251);
//	}
//	else if(FREQ >= FREQ_3700MHz && FREQ < FREQ_4400MHz){
//		Status = SetAdrvGain(&tal, 250);
//	}
//	else if(FREQ >= FREQ_4400MHz && FREQ < FREQ_5700MHz){
//		Status = SetAdrvGain(&tal, 251);
//	}
//	else if(FREQ >= FREQ_5700MHz && FREQ < FREQ_5800MHz){
//		Status = SetAdrvGain(&tal, 250);
//	}
//	else if(FREQ >= FREQ_5800MHz && FREQ < FREQ_5850MHz){
//		Status = SetAdrvGain(&tal, 248);
//	}
//	else if(FREQ >= FREQ_5850MHz && FREQ < FREQ_5900MHz){
//		Status = SetAdrvGain(&tal, 247);
//	}
//	else if(FREQ >= FREQ_5900MHz && FREQ <= FREQ_6000MHz){
//		Status = SetAdrvGain(&tal, 248);
//	}


	//1�� ����
	if(FREQ >= FREQ_400MHz && FREQ < FREQ_500MHz){
		Status = SetAdrvGain(&tal, 253);
	}
	else if(FREQ >= FREQ_500MHz && FREQ < FREQ_1000MHz){
		Status = SetAdrvGain(&tal, 255);
	}
	else if(FREQ >= FREQ_1000MHz && FREQ < FREQ_1500MHz){
		Status = SetAdrvGain(&tal, 253);
	}
	else if(FREQ >= FREQ_1500MHz && FREQ < FREQ_2000MHz){
		Status = SetAdrvGain(&tal, 251);
	}
	else if(FREQ >= FREQ_2000MHz && FREQ < FREQ_2500MHz){
		Status = SetAdrvGain(&tal, 255);
	}
	else if(FREQ >= FREQ_2500MHz && FREQ < FREQ_3700MHz){
		Status = SetAdrvGain(&tal, 251);
	}
	else if(FREQ >= FREQ_3700MHz && FREQ < FREQ_4400MHz){
		Status = SetAdrvGain(&tal, 250);
	}
	else if(FREQ >= FREQ_4400MHz && FREQ < FREQ_5700MHz){
		Status = SetAdrvGain(&tal, 247);
	}
	else if(FREQ >= FREQ_5700MHz && FREQ < FREQ_5800MHz){
		Status = SetAdrvGain(&tal, 251);
	}
	else if(FREQ >= FREQ_5800MHz && FREQ < FREQ_5850MHz){
		Status = SetAdrvGain(&tal, 249);
	}
	else if(FREQ >= FREQ_5850MHz && FREQ < FREQ_5900MHz){
		Status = SetAdrvGain(&tal, 248);
	}
	else if(FREQ >= FREQ_5900MHz && FREQ <= FREQ_6000MHz){
		Status = SetAdrvGain(&tal, 248);
	}
	else{
		Status = SetAdrvGain(&tal, 255);
		printf("AdrvGainCtrl - Frequency is out of range.\r\n");
	}

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	else{
		return XST_SUCCESS;
	}
}


int SetAdrvGain(taliseDevice_t * const pd, uint8_t gain){
	uint32_t talAction = TALACT_NO_ACTION;

	talAction = TALISE_setRxManualGain(pd, TAL_RX1, gain);
	if (talAction != TALACT_NO_ACTION) {
		printf("error: TALISE_radioOn() failed\n");
		return 1;
	}
	return 0;
}
