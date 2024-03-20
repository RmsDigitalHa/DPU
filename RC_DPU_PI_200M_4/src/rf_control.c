/*
 * bd_status.c
 *
 *  Created on: 2023. 12. 27.
 *      Author: RMS_Digital
 */

#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include "icd_parser.h"
#include "xparameters.h"
#include "xgpio.h"
#include "xstatus.h"
#include "axi_rc_spectrum.h"
#include "sleep.h"

#include "user_func.h"
#include "adrv9008.h"
#include "rf_control.h"
#include "Init.h"
#include "rcfm_control.h"
#include "rcrm_control.h"

extern HW_CHECK BIT_STATUS;
extern HW_CHECK PBIT_STATUS;
extern XGpio RF_GPIO; /* The Instance of the GPIO Driver */
extern XGpio PWR_GPIO; /* The Instance of the GPIO Driver */

extern RCFM_MODE rcfm_status;
extern RCRM_MODE rcrm_status;
extern RF_SETTING RF_STATUS;
extern RECV_SETTING DPU_STATUS;


//  0   ADC_LD
//  1   SYN_SDO
//  2   LNA_ALM1
//  3   OCXO_CTRL
//  4   PWR_FAULT1
//  5   INSERT_FAULT1
//  6   LNA_ALM2
//  7   LNA_ALM3
//  8   PWR_FAULT2
//  9   INSERT_FAULT2

//  0   PG4C
//  1   PG1D
//  2   PG3D
//  3   PG4D
//  4   PG4B
//  5   PG1B
//  6   PG1C
//  7   PG3C
//  8   PG1A
//  9   PG1F

void GetStatusPBIT(){
	int Status = 0;
	uint32_t Buf_Read = 0;
	uint8_t	Data_PBIT[10] = {0, };

	SetRcfmStatAmpFst(RCFM_LNA);
	SetRcrmStatAmpScd(RCRM_LNA02);

	usleep(1000);

	Buf_Read = XGpio_DiscreteRead(&RF_GPIO, RF_GPIO_IN);

	SetRcfmStatAmpFst(RF_STATUS.RCFM_LNA_MODE);
	SetRcrmStatAmpScd(RF_STATUS.RCRM_LNA_MODE);

	PBIT_STATUS.RCFM_TMP = GetRFTmp(TMP_RCFM_DEV);
	PBIT_STATUS.RCRM_TMP = GetRFTmp(TMP_RCRM_DEV);

	Status = GetRFPathStatus();
	if (Status != XST_SUCCESS) {
		PBIT_STATUS.RF_PATH = 1;
		PBIT_STATUS.LOCK_BIT = 1;
	}
	else{
		PBIT_STATUS.RF_PATH = 0;
		PBIT_STATUS.LOCK_BIT = 0;
	}


	for(int i = 0; i<10; i ++){
		Data_PBIT[i] = Buf_Read & (0x1U);
		Buf_Read = Buf_Read >> 1;
	}

	PBIT_STATUS.LOCK_ADCLK = (~Data_PBIT[0]) & (0x01U);
	PBIT_STATUS.LNA1 = Data_PBIT[2];
	PBIT_STATUS.REF_SIG = Data_PBIT[3];
	PBIT_STATUS.RCFM_PWR = (~Data_PBIT[4]) & (0x01U);
	PBIT_STATUS.RCFM_INSERT = Data_PBIT[5];
	PBIT_STATUS.LNA2 = Data_PBIT[6];
	PBIT_STATUS.LNA3 = Data_PBIT[7];
	PBIT_STATUS.RCRM_PWR = (~Data_PBIT[8]) & (0x01U);
	PBIT_STATUS.RCRM_INSERT = Data_PBIT[9];
	PBIT_STATUS.DONE_FPGA = 0;		//Add status check function
	PBIT_STATUS.DONE_ADC = 0;		//Add status check function
	PBIT_STATUS.DONE_DDR = 0;		//Add status check function
}


void GetStatusIBIT(){
	int Status = 0;
	uint32_t Buf_Read = 0;
	uint8_t	Data_IBIT[10] = {0, };

	SetRcfmStatAmpFst(RCFM_LNA);
	SetRcrmStatAmpScd(RCRM_LNA02);

	usleep(1000);

	Buf_Read = XGpio_DiscreteRead(&RF_GPIO, RF_GPIO_IN);

	BIT_STATUS.RCFM_TMP = GetRFTmp(TMP_RCFM_DEV);
	BIT_STATUS.RCRM_TMP = GetRFTmp(TMP_RCRM_DEV);

	SetRcfmStatAmpFst(RCFM_BYPASS);
	SetRcrmStatAmpScd(RCRM_BYPASS02);

	Status = GetRFPathStatus();
	if (Status != XST_SUCCESS) {
		BIT_STATUS.RF_PATH = 1;
		BIT_STATUS.LOCK_BIT = 1;
	}
	else{
		BIT_STATUS.RF_PATH = 0;
		BIT_STATUS.LOCK_BIT = 0;
	}


	for(int i = 0; i<10; i ++){
		Data_IBIT[i] = Buf_Read & (0x1U);
		Buf_Read = Buf_Read >> 1;
	}

	BIT_STATUS.LOCK_ADCLK = (~Data_IBIT[0]) & (0x01U);
	BIT_STATUS.LNA1 = Data_IBIT[2];
	BIT_STATUS.REF_SIG = Data_IBIT[3];
	BIT_STATUS.RCFM_PWR = (~Data_IBIT[4]) & (0x01U);
	BIT_STATUS.RCFM_INSERT = Data_IBIT[5];
	BIT_STATUS.LNA2 = Data_IBIT[6];
	BIT_STATUS.LNA3 = Data_IBIT[7];
	BIT_STATUS.RCRM_PWR = (~Data_IBIT[8]) & (0x01U);
	BIT_STATUS.RCRM_INSERT = Data_IBIT[9];
	BIT_STATUS.DONE_FPGA = 0;
	BIT_STATUS.DONE_ADC = 0;
	BIT_STATUS.DONE_DDR = 0;

	SetRcfmStatAmpFst(RF_STATUS.RCFM_LNA_MODE);
	SetRcrmStatAmpScd(RF_STATUS.RCRM_LNA_MODE);
}


/*
 * BIT Enable
 * BIT Freq_1 Set
 * RF Log IC Set
 * Check Log Level
 * BIT Freq_2 Set
 * RF Log IC Set
 * Check Log Level
 * ... (ALL Filter Path)
 * BIT Disable
 */
int GetRFPathStatus(){
	uint16_t LogValue;
	SetRcfmStatBitEn(RCFM_CAL_EN);
	SetRcfmStatPathANT(RCFM_ANT_BIAS_OFF);
	SetRcfmStatPath(RCFM_BIT_PATH);
	Init_BIT_PLL();

	SetRcrmStatFreq(FREQ_450MHz);
	SetRcfmStatBitFreq(FREQ_450MHz);

	usleep(50);
	LogValue = SPI_ReadReg(DPU_LOG, 0, 2);
	if(LogValue < 1500U){
		printf("450MHz Log Level : %d\r\n", LogValue);
		return -1;
	}
	else{}

	//Consider Timing Margin
	SetRcrmStatFreq(FREQ_610MHz);
	SetRcfmStatBitFreq(FREQ_610MHz);

	usleep(50);
	LogValue = SPI_ReadReg(DPU_LOG, 0, 2);
	if(LogValue < 1500U){
		printf("610MHz Log Level : %d\r\n", LogValue);
		return -1;
	}
	else{}

	SetRcrmStatFreq(FREQ_830MHz);
	SetRcfmStatBitFreq(FREQ_830MHz);

	usleep(50);
	LogValue = SPI_ReadReg(DPU_LOG, 0, 2);
	if(LogValue < 1500U){
		printf("830MHz Log Level : %d\r\n", LogValue);
		return -1;
	}
	else{}

	SetRcrmStatFreq(FREQ_1120MHz);
	SetRcfmStatBitFreq(FREQ_1120MHz);

	usleep(50);
	LogValue = SPI_ReadReg(DPU_LOG, 0, 2);
	if(LogValue < 1500U){
		printf("1120MHz Log Level : %d\r\n", LogValue);
		return -1;
	}
	else{}

	SetRcrmStatFreq(FREQ_1500MHz);
	SetRcfmStatBitFreq(FREQ_1500MHz);

	usleep(50);
	LogValue = SPI_ReadReg(DPU_LOG, 0, 2);
	if(LogValue < 1500U){
		printf("1500MHz Log Level : %d\r\n", LogValue);
		return -1;
	}
	else{}

	SetRcrmStatFreq(FREQ_2100MHz);
	SetRcfmStatBitFreq(FREQ_2100MHz);

	usleep(50);
	LogValue = SPI_ReadReg(DPU_LOG, 0, 2);
	if(LogValue < 1500U){
		printf("2100MHz Log Level : %d\r\n", LogValue);
		return -1;
	}
	else{}

	SetRcrmStatFreq(FREQ_4850MHz);
	SetRcfmStatBitFreq(FREQ_4850MHz);

	usleep(50);
	LogValue = SPI_ReadReg(DPU_LOG, 0, 2);
	if(LogValue < 1500U){
		printf("4850MHz Log Level : %d\r\n", LogValue);
		return -1;
	}
	else{}

	//Restore Default Value
	SetRcfmStatBitEn(RF_STATUS.RF_BIT_EN);
	SetRcrmStatFreq(DPU_STATUS.CenterFreq);
	SetRcfmStatPath(RF_STATUS.RCV_PATH);
	SetRcfmStatPathANT(RF_STATUS.ANT_PATH);

	return XST_SUCCESS;
}


void SetGainAtten(uint64_t Freq){
	uint8_t		AmpFstAtten = 0;
	uint8_t		AmpScdAtten = 0;
	uint8_t		BypassAtten = 0;
	uint8_t		SysAtten = 0;
	typTableATTEN ParamGainAtten = {0,};


	if(Freq >= MIN_Freq && Freq <= MAX_Freq){
		ParamGainAtten = GetAttenValue(Freq);

		AmpFstAtten  = ParamGainAtten.u8AMP1_GAIN_ATTEN;
		AmpScdAtten  = ParamGainAtten.u8AMP2_GAIN_ATTEN;
		BypassAtten  = ParamGainAtten.u8BYPASS_GAIN_ATTEN;
		SysAtten	 = ParamGainAtten.u8SYSTEM_ATTEN;

		if((rcfm_status.rcfm_amp_mode1 == 0x00U) & (rcrm_status.rcrm_amp_mode2 == 0x00U)){	//BYPASS Mode
			rcrm_status.rcrm_gain_att = BypassAtten;
		}
		else if((rcfm_status.rcfm_amp_mode1 == 0x00U) & (rcrm_status.rcrm_amp_mode2 == 0x01U)){	//AMP1 Mode
			rcrm_status.rcrm_gain_att = AmpFstAtten;
		}
		else if((rcfm_status.rcfm_amp_mode1 == 0x01U) & (rcrm_status.rcrm_amp_mode2 == 0x01U)){	//AMP2 Mode
			rcrm_status.rcrm_gain_att = AmpScdAtten;
		}

		//Common
		rcrm_status.rcrm_sys_att = SysAtten;
	}

	else{
		printf("Frequency setting is out of range.\n");
	}
}


typTableATTEN GetAttenValue(uint64_t TargetFreq){
	int32_t MinIndex = 0;
	int32_t MaxIndex = 0;
	int32_t AvgIndex = 0;
	int32_t OldAvgIndex = -1;
	typTableATTEN ParamGainAtten = {0,};

	MaxIndex = G_ATTEN_TABLE_SIZE;
	AvgIndex = (MinIndex + MaxIndex) / 2;
	OldAvgIndex = -1;

	if (TargetFreq == GainAttenTable[MaxIndex-1].u64StartFreq)
	{
		AvgIndex = MaxIndex;
		memcpy(&ParamGainAtten, &GainAttenTable[AvgIndex], sizeof(typTableATTEN));
		return ParamGainAtten;
	}

	while(1)
	{
		if((TargetFreq >= GainAttenTable[AvgIndex].u64StartFreq) && (TargetFreq < GainAttenTable[AvgIndex + 1].u64StartFreq))
		{
			memcpy(&ParamGainAtten, &GainAttenTable[AvgIndex], sizeof(typTableATTEN));
			return ParamGainAtten;
		}

		if(TargetFreq > GainAttenTable[AvgIndex].u64StartFreq)
		{
			MinIndex = AvgIndex;
			AvgIndex = (int32_t)((MinIndex + MaxIndex)/2);
		}
		else
		{
			MaxIndex = AvgIndex;
			AvgIndex = (int32_t)((MinIndex + MaxIndex)/2);
		}

		if(AvgIndex == OldAvgIndex)
		{
			break;
		}
		else
			OldAvgIndex = AvgIndex;
	}
	return ParamGainAtten;
}


typTableATTEN GainAttenTable[167] =
{
	//Frequency	   AMP1    AMP2	  BYPASS   SYS
	{400000000,		25,		26,		23,	  0},
	{410000000,		25,		26,		23,   0},
	{420000000,		25,		26,		23,   0},
	{430000000,		25,		26,		22,   0},
	{440000000,		24,		26,		22,   0},
	{450000000,		23,		26,		22,   0},
	{460000000,		23,		25,		22,   0},
	{470000000,		23,		25,		23,   0},
	{480000000,		23,		24,		22,   0},
	{490000000,		23,		24,		22,   0},
	{500000000,		23,		24,		22,   0},
	{500000000,		21,		21,		21,   0},
	{520000000,		21,		22,		21,   0},
	{540000000,		21,		22,		21,   0},
	{560000000,		21,		22,		20,   0},
	{580000000,		20,		22,		20,   0},
	{600000000,		20,		21,		20,   0},
	{620000000,		20,		21,		20,   0},
	{640000000,		20,		21,		20,   0},
	{660000000,		20,		21,		20,   0},
	{680000000,		20,		22,		19,   0},
	{700000000,		20,		21,		18,   0},
	{720000000,		20,		21,		18,   0},
	{720000000,		21,		22,		20,   0},
	{740000000,		19,		21,		19,   0},
	{760000000,		19,		21,		19,   0},
	{780000000,		18,		20,		18,   0},
	{800000000,		19,		20,		18,   0},
	{820000000,		19,		19,		18,   0},
	{840000000,		18,		19,		18,   0},
	{860000000,		19,		19,		18,   0},
	{880000000,		19,		19,		18,   0},
	{900000000,		19,		19,		18,   0},
	{920000000,		19,		19,		18,   0},
	{940000000,		19,		20,		18,   0},
	{940000000,		19,		19,		18,   0},
	{960000000,		19,		19,		18,   0},
	{980000000,		18,		19,		18,   0},
	{1000000000,	19,		19,		18,   0},
	{1020000000,	18,		19,		18,   0},
	{1040000000,	19,		20,		18,   0},
	{1060000000,	18,		19,		18,   0},
	{1080000000,	18,		19,		18,   0},
	{1100000000,	18,		19,		17,   0},
	{1120000000,	17,		19,		17,   0},
	{1140000000,	17,		18,		17,   0},
	{1160000000,	17,		18,		17,   0},
	{1180000000,	18,		18,		17,   0},
	{1200000000,	18,		18,		17,   0},
	{1220000000,	17,		18,		16,   0},
	{1240000000,	17,		18,		16,   0},
	{1260000000,	16,		17,		16,   0},
	{1280000000,	16,		17,		16,   0},
	{1300000000,	15,		16,		15,   0},
	{1300000000,	17,		17,		17,   0},
	{1320000000,	17,		17,		17,   0},
	{1340000000,	18,		16,		17,   0},
	{1360000000,	18,		16,		18,   0},
	{1380000000,	18,		16,		17,   0},
	{1400000000,	17,		17,		17,   0},
	{1420000000,	17,		17,		17,   0},
	{1440000000,	17,		17,		16,   0},
	{1460000000,	16,		16,		16,   0},
	{1480000000,	16,		16,		16,   0},
	{1500000000,	16,		15,		16,   0},
	{1520000000,	15,		15,		15,   0},
	{1540000000,	15,		15,		15,   0},
	{1560000000,	14,		15,		15,   0},
	{1580000000,	14,		15,		15,   0},
	{1600000000,	14,		15,		15,   0},
	{1620000000,	15,		15,		15,   0},
	{1640000000,	15,		15,		15,   0},
	{1660000000,	15,		15,		15,   0},
	{1680000000,	15,		15,		15,   0},
	{1700000000,	15,		15,		15,   0},
	{1700000000,	14,		15,		13,   0},
	{1740000000,	14,		14,		13,   0},
	{1780000000,	14,		14,		13,   0},
	{1820000000,	14,		14,		13,   0},
	{1860000000,	14,		13,		13,   0},
	{1900000000,	13,		13,		13,   0},
	{1940000000,	13,		12,		13,   0},
	{1980000000,	13,		12,		14,   0},
	{2020000000,	13,		13,		14,   0},
	{2060000000,	13,		13,		14,   0},
	{2100000000,	13,		14,		14,   0},
	{2140000000,	14,		14,		14,   0},
	{2180000000,	13,		14,		13,   0},
	{2220000000,	12,		12,		12,   0},
	{2260000000,	12,		11,		12,   0},
	{2300000000,	11,		11,		12,   0},
	{2340000000,	11,		10,		12,   0},
	{2380000000,	11,		11,		12,   0},
	{2420000000,	12,		11,		12,   0},
	{2460000000,	13,		12,		13,   0},
	{2500000000,	13,		12,		13,   0},
	{2500000000,	18,		17,		18,   0},
	{2540000000,	17,		17,		18,   0},
	{2580000000,	17,		17,		18,   0},
	{2620000000,	18,		17,		18,   0},
	{2660000000,	18,		17,		19,   0},
	{2700000000,	18,		16,		19,   0},
	{2740000000,	18,		16,		18,   0},
	{2780000000,	16,		15,		17,   0},
	{2820000000,	15,		14,		16,   0},
	{2860000000,	14,		13,		15,   0},
	{2900000000,	13,		11,		14,   0},
	{2940000000,	12,		10,		14,   0},
	{2980000000,	11,		8 ,		13,   0},
	{3020000000,	9 ,		7 ,		12,   0},
	{3060000000,	7 ,		6 ,		10,   0},
	{3100000000,	7 ,		6 ,		9 ,   0},
	{3140000000,	8 ,		6 ,		9 ,   0},
	{3180000000,	8 ,		6 ,		10,   0},
	{3220000000,	10,		8 ,		11,   0},
	{3260000000,	10,		10,		11,   0},
	{3300000000,	9 ,		10,		11,   0},
	{3340000000,	8 ,		9 ,		11,   0},
	{3380000000,	9 ,		8 ,		11,   0},
	{3420000000,	9 ,		8 ,		12,   0},
	{3460000000,	10,		8 ,		12,   0},
	{3500000000,	10,		9 ,		12,   0},
	{3540000000,	11,		11,		12,   0},
	{3580000000,	12,		11,		13,   0},
	{3620000000,	12,		11,		13,   0},
	{3660000000,	11,		11,		13,   0},
	{3700000000,	11,		11,		13,   0},
	{3700000000,	12,		11,		13,   0},
	{3760000000,	11,		10,		12,   0},
	{3820000000,	10,		9 ,		11,   0},
	{3880000000,	8 ,		6 ,		9 ,   0},
	{3940000000,	5 ,		4 ,		7 ,   0},
	{4000000000,	5 ,		5 ,		7 ,   0},
	{4060000000,	7 ,		8 ,		9 ,   0},
	{4120000000,	10,		9 ,		11,   0},
	{4180000000,	10,		8 ,		11,   0},
	{4240000000,	8 ,		7 ,		9 ,   0},
	{4300000000,	7 ,		6 ,		9 ,   0},
	{4360000000,	7 ,		6 ,		8 ,   0},
	{4420000000,	6 ,		6 ,		7 ,   0},
	{4480000000,	5 ,		6 ,		7 ,   0},
	{4540000000,	5 ,		6 ,		7 ,   0},
	{4600000000,	6 ,		6 ,		6 ,   0},
	{4660000000,	7 ,		6 ,		8 ,   0},
	{4720000000,	7 ,		6 ,		7 ,   0},
	{4780000000,	2 ,		2 ,		3 ,   0},
	{4840000000,	0 ,		0 ,		1 ,   0},
	{4900000000,	2 ,		2 ,		4 ,   0},
	{4960000000,	6 ,		5 ,		7 ,   0},
	{5020000000,	8 ,		6 ,		9 ,   0},
	{5080000000,	7 ,		6 ,		8 ,   0},
	{5140000000,	6 ,		5 ,		7 ,   0},
	{5200000000,	5 ,		4 ,		6 ,   0},
	{5260000000,	4 ,		3 ,		5 ,   0},
	{5320000000,	1 ,		1 ,		3 ,   0},
	{5380000000,	0 ,		0 ,		0 ,   0},
	{5440000000,	0 ,		0 ,		0 ,   0},
	{5500000000,	1 ,		0 ,		3 ,   0},
	{5560000000,	3 ,		0 ,		4 ,   0},
	{5620000000,	2 ,		0 ,		4 ,   0},
	{5680000000,	2 ,		0 ,		4 ,   0},
	{5740000000,	2 ,		0 ,		3 ,   0},
	{5800000000,	0 ,		0 ,		2 ,   0},
	{5860000000,	0 ,		0 ,		0 ,   0},
	{5920000000,	0 ,		0 ,		0 ,   0},
	{5980000000,	0 ,		0 ,		2 ,   0},
	{6040000000,	1 ,		0 ,		3 ,   0}
};
