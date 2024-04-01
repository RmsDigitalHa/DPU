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
#include <stdbool.h>

#include "user_func.h"
#include "adrv9008.h"
#include "rf_control.h"
#include "Init.h"
#include "rcfm_control.h"
#include "rcrm_control.h"

extern XGpio RF_GPIO; /* The Instance of the GPIO Driver */
extern XGpio PWR_GPIO; /* The Instance of the GPIO Driver */

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

void GetStatusPBIT(void){
	int Status = 0;
	uint32_t Buf_Read = 0;
	uint8_t	Data_PBIT[10] = {0,0,0,0,0,0,0,0,0,0};

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
		PBIT_STATUS.RF_PATH = 1U;
		PBIT_STATUS.LOCK_BIT = 1U;
	}
	else{
		PBIT_STATUS.RF_PATH = 0U;
		PBIT_STATUS.LOCK_BIT = 0U;
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


void GetStatusIBIT(void){
	int Status = 0;
	uint32_t Buf_Read = 0;
	uint8_t	Data_IBIT[10] = {0,0,0,0,0,0,0,0,0,0};

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
		BIT_STATUS.RF_PATH = 1U;
		BIT_STATUS.LOCK_BIT = 1U;
	}
	else{
		BIT_STATUS.RF_PATH = 0U;
		BIT_STATUS.LOCK_BIT = 0U;
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
static int GetRFPathStatus(void){
	uint16_t LogValue;
	SetRcfmStatBitEn(RCFM_CAL_EN);
	SetRcfmStatPathANT(RCFM_ANT_BIAS_OFF);
	SetRcfmStatPath(RCFM_BIT_PATH);
	Init_BIT_PLL();

	SetRcrmStatFreq(FREQ_450MHz);
	SetRcfmStatBitFreq(FREQ_450MHz);

	usleep(50);
	LogValue = SPI_ReadReg(DPU_LOG, 0U, 2U);
	if(LogValue < 1500U){
		printf("450MHz Log Level : %d\r\n", LogValue);
		return -1;
	}
	else{}

	//Consider Timing Margin
	SetRcrmStatFreq(FREQ_610MHz);
	SetRcfmStatBitFreq(FREQ_610MHz);

	usleep(50);
	LogValue = SPI_ReadReg(DPU_LOG, 0U, 2U);
	if(LogValue < 1500U){
		printf("610MHz Log Level : %d\r\n", LogValue);
		return -1;
	}
	else{}

	SetRcrmStatFreq(FREQ_830MHz);
	SetRcfmStatBitFreq(FREQ_830MHz);

	usleep(50);
	LogValue = SPI_ReadReg(DPU_LOG, 0U, 2U);
	if(LogValue < 1500U){
		printf("830MHz Log Level : %d\r\n", LogValue);
		return -1;
	}
	else{}

	SetRcrmStatFreq(FREQ_1120MHz);
	SetRcfmStatBitFreq(FREQ_1120MHz);

	usleep(50);
	LogValue = SPI_ReadReg(DPU_LOG, 0U, 2U);
	if(LogValue < 1500U){
		printf("1120MHz Log Level : %d\r\n", LogValue);
		return -1;
	}
	else{}

	SetRcrmStatFreq(FREQ_1500MHz);
	SetRcfmStatBitFreq(FREQ_1500MHz);

	usleep(50);
	LogValue = SPI_ReadReg(DPU_LOG, 0U, 2U);
	if(LogValue < 1500U){
		printf("1500MHz Log Level : %d\r\n", LogValue);
		return -1;
	}
	else{}

	SetRcrmStatFreq(FREQ_2100MHz);
	SetRcfmStatBitFreq(FREQ_2100MHz);

	usleep(50);
	LogValue = SPI_ReadReg(DPU_LOG, 0U, 2U);
	if(LogValue < 1500U){
		printf("2100MHz Log Level : %d\r\n", LogValue);
		return -1;
	}
	else{}

	SetRcrmStatFreq(FREQ_4850MHz);
	SetRcfmStatBitFreq(FREQ_4850MHz);

	usleep(50);
	LogValue = SPI_ReadReg(DPU_LOG, 0U, 2U);
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


void SetGainAtten(const uint64_t Freq){
	uint8_t		AmpFstAtten = 0;
	uint8_t		AmpScdAtten = 0;
	uint8_t		BypassAtten = 0;
	uint8_t		SysAtten = 0;
	typTableATTEN ParamGainAtten = {0,};


	if((Freq >= MIN_Freq) && (Freq <= MAX_Freq)){
		ParamGainAtten = GetAttenValue(Freq);

		AmpFstAtten  = ParamGainAtten.u8AMP1_GAIN_ATTEN;
		AmpScdAtten  = ParamGainAtten.u8AMP2_GAIN_ATTEN;
		BypassAtten  = ParamGainAtten.u8BYPASS_GAIN_ATTEN;
		SysAtten	 = ParamGainAtten.u8SYSTEM_ATTEN;

		if((rcfm_status.rcfm_amp_mode1 == 0x00U) && (rcrm_status.rcrm_amp_mode2 == 0x00U)){	//BYPASS Mode
			rcrm_status.rcrm_gain_att = BypassAtten;
		}
		else if((rcfm_status.rcfm_amp_mode1 == 0x00U) && (rcrm_status.rcrm_amp_mode2 == 0x01U)){	//AMP1 Mode
			rcrm_status.rcrm_gain_att = AmpFstAtten;
		}
		else if((rcfm_status.rcfm_amp_mode1 == 0x01U) && (rcrm_status.rcrm_amp_mode2 == 0x01U)){	//AMP2 Mode
			rcrm_status.rcrm_gain_att = AmpScdAtten;
		}
		else { }

		//Common
		rcrm_status.rcrm_sys_att = SysAtten;
	}

	else{
		printf("Frequency setting is out of range.\n");
	}
}


static typTableATTEN GetAttenValue(const uint64_t TargetFreq){
	uint32_t MinIndex = 0;
	uint32_t MaxIndex = 0;
	int32_t AvgIndex = 0;
	int32_t OldAvgIndex = -1;
	typTableATTEN ParamGainAtten = {0,};

	MaxIndex = G_ATTEN_TABLE_SIZE;
	AvgIndex = ((int32_t)MinIndex + (int32_t)MaxIndex) / (int32_t)2;
	OldAvgIndex = -1;

	if (TargetFreq == GainAttenTable[MaxIndex-(uint32_t)1].u64StartFreq)
	{
		AvgIndex = (int32_t)MaxIndex;
		(void)memcpy(&ParamGainAtten, &GainAttenTable[AvgIndex], sizeof(typTableATTEN));
		return ParamGainAtten;
	}

	while(true)
	{
		if((TargetFreq >= GainAttenTable[AvgIndex].u64StartFreq) && (TargetFreq < GainAttenTable[AvgIndex + 1].u64StartFreq))
		{
			(void)memcpy(&ParamGainAtten, &GainAttenTable[AvgIndex], sizeof(typTableATTEN));
			return ParamGainAtten;
		}

		if(TargetFreq > GainAttenTable[AvgIndex].u64StartFreq)
		{
			MinIndex = (uint32_t)AvgIndex;
			AvgIndex = ((int32_t)MinIndex + (int32_t)MaxIndex) / (int32_t)2;
		}
		else
		{
			MaxIndex = (uint32_t)AvgIndex;
			AvgIndex = ((int32_t)MinIndex +(int32_t)MaxIndex) / (int32_t)2;
		}

		if(AvgIndex == OldAvgIndex)
		{
			break;
		}
		else {
			OldAvgIndex = AvgIndex;
		}
	}
	return ParamGainAtten;
}


typTableATTEN GainAttenTable[167] =
{
	//Frequency	   AMP1    AMP2	  BYPASS   SYS
	{400000000U,	25U,	26U,	23U,    0U},
	{410000000U,	25U,	26U,	23U,    0U},
	{420000000U,	25U,	26U,	23U,    0U},
	{430000000U,	25U,	26U,	22U,    0U},
	{440000000U,	24U,	26U,	22U,    0U},
	{450000000U,	23U,	26U,	22U,    0U},
	{460000000U,	23U,	25U,	22U,    0U},
	{470000000U,	23U,	25U,	23U,    0U},
	{480000000U,	23U,	24U,	22U,    0U},
	{490000000U,	23U,	24U,	22U,    0U},
	{500000000U,	23U,	24U,	22U,    0U},
	{500000000U,	21U,	21U,	21U,    0U},
	{520000000U,	21U,	22U,	21U,    0U},
	{540000000U,	21U,	22U,	21U,    0U},
	{560000000U,	21U,	22U,	20U,    0U},
	{580000000U,	20U,	22U,	20U,    0U},
	{600000000U,	20U,	21U,	20U,    0U},
	{620000000U,	20U,	21U,	20U,    0U},
	{640000000U,	20U,	21U,	20U,    0U},
	{660000000U,	20U,	21U,	20U,    0U},
	{680000000U,	20U,	22U,	19U,    0U},
	{700000000U,	20U,	21U,	18U,    0U},
	{720000000U,	20U,	21U,	18U,    0U},
	{720000000U,	21U,	22U,	20U,    0U},
	{740000000U,	19U,	21U,	19U,    0U},
	{760000000U,	19U,	21U,	19U,    0U},
	{780000000U,	18U,	20U,	18U,    0U},
	{800000000U,	19U,	20U,	18U,    0U},
	{820000000U,	19U,	19U,	18U,    0U},
	{840000000U,	18U,	19U,	18U,    0U},
	{860000000U,	19U,	19U,	18U,    0U},
	{880000000U,	19U,	19U,	18U,    0U},
	{900000000U,	19U,	19U,	18U,    0U},
	{920000000U,	19U,	19U,	18U,    0U},
	{940000000U,	19U,	20U,	18U,    0U},
	{940000000U,	19U,	19U,	18U,    0U},
	{960000000U,	19U,	19U,	18U,    0U},
	{980000000U,	18U,	19U,	18U,    0U},
	{1000000000U,	19U,	19U,	18U,    0U},
	{1020000000U,	18U,	19U,	18U,    0U},
	{1040000000U,	19U,	20U,	18U,    0U},
	{1060000000U,	18U,	19U,	18U,    0U},
	{1080000000U,	18U,	19U,	18U,    0U},
	{1100000000U,	18U,	19U,	17U,    0U},
	{1120000000U,	17U,	19U,	17U,    0U},
	{1140000000U,	17U,	18U,	17U,    0U},
	{1160000000U,	17U,	18U,	17U,    0U},
	{1180000000U,	18U,	18U,	17U,    0U},
	{1200000000U,	18U,	18U,	17U,    0U},
	{1220000000U,	17U,	18U,	16U,    0U},
	{1240000000U,	17U,	18U,	16U,    0U},
	{1260000000U,	16U,	17U,	16U,    0U},
	{1280000000U,	16U,	17U,	16U,    0U},
	{1300000000U,	15U,	16U,	15U,    0U},
	{1300000000U,	17U,	17U,	17U,    0U},
	{1320000000U,	17U,	17U,	17U,    0U},
	{1340000000U,	18U,	16U,	17U,    0U},
	{1360000000U,	18U,	16U,	18U,    0U},
	{1380000000U,	18U,	16U,	17U,    0U},
	{1400000000U,	17U,	17U,	17U,    0U},
	{1420000000U,	17U,	17U,	17U,    0U},
	{1440000000U,	17U,	17U,	16U,    0U},
	{1460000000U,	16U,	16U,	16U,    0U},
	{1480000000U,	16U,	16U,	16U,    0U},
	{1500000000U,	16U,	15U,	16U,    0U},
	{1520000000U,	15U,	15U,	15U,    0U},
	{1540000000U,	15U,	15U,	15U,    0U},
	{1560000000U,	14U,	15U,	15U,    0U},
	{1580000000U,	14U,	15U,	15U,    0U},
	{1600000000U,	14U,	15U,	15U,    0U},
	{1620000000U,	15U,	15U,	15U,    0U},
	{1640000000U,	15U,	15U,	15U,    0U},
	{1660000000U,	15U,	15U,	15U,    0U},
	{1680000000U,	15U,	15U,	15U,    0U},
	{1700000000U,	15U,	15U,	15U,    0U},
	{1700000000U,	14U,	15U,	13U,    0U},
	{1740000000U,	14U,	14U,	13U,    0U},
	{1780000000U,	14U,	14U,	13U,    0U},
	{1820000000U,	14U,	14U,	13U,    0U},
	{1860000000U,	14U,	13U,	13U,    0U},
	{1900000000U,	13U,	13U,	13U,    0U},
	{1940000000U,	13U,	12U,	13U,    0U},
	{1980000000U,	13U,	12U,	14U,    0U},
	{2020000000U,	13U,	13U,	14U,    0U},
	{2060000000U,	13U,	13U,	14U,    0U},
	{2100000000U,	13U,	14U,	14U,    0U},
	{2140000000U,	14U,	14U,	14U,    0U},
	{2180000000U,	13U,	14U,	13U,    0U},
	{2220000000U,	12U,	12U,	12U,    0U},
	{2260000000U,	12U,	11U,	12U,    0U},
	{2300000000U,	11U,	11U,	12U,    0U},
	{2340000000U,	11U,	10U,	12U,    0U},
	{2380000000U,	11U,	11U,	12U,    0U},
	{2420000000U,	12U,	11U,	12U,    0U},
	{2460000000U,	13U,	12U,	13U,    0U},
	{2500000000U,	13U,	12U,	13U,    0U},
	{2500000000U,	18U,	17U,	18U,    0U},
	{2540000000U,	17U,	17U,	18U,    0U},
	{2580000000U,	17U,	17U,	18U,    0U},
	{2620000000U,	18U,	17U,	18U,    0U},
	{2660000000U,	18U,	17U,	19U,    0U},
	{2700000000U,	18U,	16U,	19U,    0U},
	{2740000000U,	18U,	16U,	18U,    0U},
	{2780000000U,	16U,	15U,	17U,    0U},
	{2820000000U,	15U,	14U,	16U,    0U},
	{2860000000U,	14U,	13U,	15U,    0U},
	{2900000000U,	13U,	11U,	14U,    0U},
	{2940000000U,	12U,	10U,	14U,    0U},
	{2980000000U,	11U,	8U,		13U,    0U},
	{3020000000U,	9U,		7U,		12U,    0U},
	{3060000000U,	7U,		6U,		10U,    0U},
	{3100000000U,	7U,		6U,		9U,    	0U},
	{3140000000U,	8U,		6U,		9U,    	0U},
	{3180000000U,	8U,		6U,		10U,    0U},
	{3220000000U,	10U,	8U,		11U,    0U},
	{3260000000U,	10U,	10U,	11U,    0U},
	{3300000000U,	9U,		10U,	11U,    0U},
	{3340000000U,	8U,		9U,		11U,    0U},
	{3380000000U,	9U,		8U,		11U,    0U},
	{3420000000U,	9U,		8U,		12U,    0U},
	{3460000000U,	10U,	8U,		12U,    0U},
	{3500000000U,	10U,	9U,		12U,    0U},
	{3540000000U,	11U,	11U,	12U,    0U},
	{3580000000U,	12U,	11U,	13U,    0U},
	{3620000000U,	12U,	11U,	13U,    0U},
	{3660000000U,	11U,	11U,	13U,    0U},
	{3700000000U,	11U,	11U,	13U,    0U},
	{3700000000U,	12U,	11U,	13U,    0U},
	{3760000000U,	11U,	10U,	12U,    0U},
	{3820000000U,	10U,	9U,		11U,    0U},
	{3880000000U,	8U,		6U,		9U,    	0U},
	{3940000000U,	5U,		4U,		7U,    	0U},
	{4000000000U,	5U,		5U,		7U,    	0U},
	{4060000000U,	7U,		8U,		9U,    	0U},
	{4120000000U,	10U,	9U,		11U,    0U},
	{4180000000U,	10U,	8U,		11U,    0U},
	{4240000000U,	8U,		7U,		9U,    	0U},
	{4300000000U,	7U,		6U,		9U,    	0U},
	{4360000000U,	7U,		6U,		8U,    	0U},
	{4420000000U,	6U,		6U,		7U,    	0U},
	{4480000000U,	5U,		6U,		7U,    	0U},
	{4540000000U,	5U,		6U,		7U,    	0U},
	{4600000000U,	6U,		6U,		6U,    	0U},
	{4660000000U,	7U,		6U,		8U,    	0U},
	{4720000000U,	7U,		6U,		7U,    	0U},
	{4780000000U,	2U,		2U,		3U,    	0U},
	{4840000000U,	0U,		0U,		1U,    	0U},
	{4900000000U,	2U,		2U,		4U,    	0U},
	{4960000000U,	6U,		5U,		7U,    	0U},
	{5020000000U,	8U,		6U,		9U,    	0U},
	{5080000000U,	7U,		6U,		8U,    	0U},
	{5140000000U,	6U,		5U,		7U,    	0U},
	{5200000000U,	5U,		4U,		6U,    	0U},
	{5260000000U,	4U,		3U,		5U,    	0U},
	{5320000000U,	1U,		1U,		3U,    	0U},
	{5380000000U,	0U,		0U,		0U,    	0U},
	{5440000000U,	0U,		0U,		0U,    	0U},
	{5500000000U,	1U,		0U,		3U,    	0U},
	{5560000000U,	3U,		0U,		4U,    	0U},
	{5620000000U,	2U,		0U,		4U,    	0U},
	{5680000000U,	2U,		0U,		4U,    	0U},
	{5740000000U,	2U,		0U,		3U,    	0U},
	{5800000000U,	0U,		0U,		2U,    	0U},
	{5860000000U,	0U,		0U,		0U,    	0U},
	{5920000000U,	0U,		0U,		0U,    	0U},
	{5980000000U,	0U,		0U,		2U,    	0U},
	{6040000000U,	1U,		0U,		3U,   	0U}
};
