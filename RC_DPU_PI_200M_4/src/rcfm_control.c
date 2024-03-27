/*
 * rcfm_control.c
 *
 *  Created on: 2023. 8. 16.
 *      Author: hwhwh
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
#include "rcfm_control.h"
#include "Init.h"
#include "rf_control.h"


static XGpio gpio_rcfm_ctrl;

RCFM_MODE rcfm_status;

/* LNA 1 CTRL(RCFM BD)
 * LNA 1 ON	 - AMP2 Mode
 * LNA 1 OFF - AMP1 Mode
 */
void SetRcfmStatAmpFst(uint8_t rcfm_amp_fst)
{
	uint32_t Old_Data = 0;
	uint32_t New_Data = 0;

	if(rcfm_amp_fst == RCFM_BYPASS) {
		rcfm_status.rcfm_amp_mode2 = 0x01;
		rcfm_status.rcfm_amp_mode1 = 0x00;
	}
	else if(rcfm_amp_fst == RCFM_LNA) {
		rcfm_status.rcfm_amp_mode2 = 0x00;
		rcfm_status.rcfm_amp_mode1 = 0x01;
	}
	else { }

	Old_Data = XGpio_DiscreteRead(&RF_GPIO, RF_GPIO_OUT);
	New_Data = (Old_Data & 0xFFFFFFF3U) | (((uint32_t)rcfm_status.rcfm_amp_mode2 & 0x1U) << 3) | (((uint32_t)rcfm_status.rcfm_amp_mode1 & 0x1U) << 2);
	XGpio_DiscreteWrite(&RF_GPIO, RF_GPIO_OUT, New_Data);
}


/*
 * RCFM_Path(SW_V1, BIT_V1)
 */
void SetRcfmStatPath(uint8_t rcfm_path)
{
	uint32_t Old_Data = 0;
	uint32_t New_Data = 0;

	if(rcfm_path == RCFM_ANT_PATH) {
		rcfm_status.rcfm_rf_select = 0x00;
	}
	else if(rcfm_path == RCFM_BIT_PATH) {
		rcfm_status.rcfm_rf_select = 0x01;
	}
	else { }

	Old_Data = XGpio_DiscreteRead(&RF_GPIO, RF_GPIO_OUT);
	New_Data = (Old_Data & 0xFFFFFFEFU) | (((~((uint32_t)rcfm_status.rcfm_rf_select)) & 0x1U) << 4);
	XGpio_DiscreteWrite(&RF_GPIO, RF_GPIO_OUT, New_Data);
}


void SetRcfmStatBitEn(uint8_t rcfm_bit_en)
{
	uint32_t Old_Data = 0;
	uint32_t New_Data = 0;

	if(rcfm_bit_en == RCFM_CAL_DIS) {
		rcfm_status.rcfm_cal_en = 0x00;
	}
	else if(rcfm_bit_en == RCFM_CAL_EN) {
		rcfm_status.rcfm_cal_en = 0x01;
	}
	else { }

	Old_Data = XGpio_DiscreteRead(&RF_GPIO, RF_GPIO_OUT);
	New_Data = (Old_Data & 0xFFFFFFBFU) | (((uint32_t)rcfm_status.rcfm_cal_en & 0x1U) << 6);
	XGpio_DiscreteWrite(&RF_GPIO, RF_GPIO_OUT, New_Data);
}


void SetRcfmStatPathANT(uint8_t rcfm_path_lna)
{
	uint32_t Old_Data = 0;
	uint32_t New_Data = 0;

	if(rcfm_path_lna == RCFM_ANT_BIAS_OFF) {
		rcfm_status.rcfm_ant_bias = 0x00;
	}
	else if(rcfm_path_lna == RCFM_ANT_BIAS_ON) {
		rcfm_status.rcfm_ant_bias = 0x01;
	}
	else { }

	Old_Data = XGpio_DiscreteRead(&RF_GPIO, RF_GPIO_OUT);
	New_Data = (Old_Data & 0xFFFFFFDFU) | (((uint32_t)rcfm_status.rcfm_ant_bias & 0x1U) << 5);
	XGpio_DiscreteWrite(&RF_GPIO, RF_GPIO_OUT, New_Data);
}


void Init_BIT_PLL(void){
	//SPI Default Value Setting
	SPI_WriteReg(LMX2592, 0x46, 0x0000, 3);
	SPI_WriteReg(LMX2592, 0x45, 0x0000, 3);
	SPI_WriteReg(LMX2592, 0x44, 0x0089, 3);
	SPI_WriteReg(LMX2592, 0x40, 0x0077, 3);
	SPI_WriteReg(LMX2592, 0x3E, 0x0000, 3);
	SPI_WriteReg(LMX2592, 0x3D, 0x0001, 3);
	SPI_WriteReg(LMX2592, 0x3B, 0x0000, 3);

	SPI_WriteReg(LMX2592, 0x30, 0X03FC, 3);
	SPI_WriteReg(LMX2592, 0x2F, 0X00CF, 3);
	SPI_WriteReg(LMX2592, 0x2E, 0X0FA3, 3);
	SPI_WriteReg(LMX2592, 0x2D, 0x0000, 3);
	SPI_WriteReg(LMX2592, 0x2C, 0x0000, 3);
	SPI_WriteReg(LMX2592, 0x2B, 0x0000, 3);
	SPI_WriteReg(LMX2592, 0x2A, 0x0000, 3);
	SPI_WriteReg(LMX2592, 0x29, 0X03E8, 3);
	SPI_WriteReg(LMX2592, 0x28, 0x0000, 3);
	SPI_WriteReg(LMX2592, 0x27, 0X8204, 3);
	SPI_WriteReg(LMX2592, 0x26, 0X0050, 3);		//DPU Version
	SPI_WriteReg(LMX2592, 0x25, 0X4000, 3);
	SPI_WriteReg(LMX2592, 0x24, 0X0421, 3);
	SPI_WriteReg(LMX2592, 0x23, 0X049B, 3);
	SPI_WriteReg(LMX2592, 0x22, 0XC3EA, 3);
	SPI_WriteReg(LMX2592, 0x21, 0X2A0A, 3);
	SPI_WriteReg(LMX2592, 0x20, 0X210A, 3);
	SPI_WriteReg(LMX2592, 0x1F, 0X0601, 3);

	SPI_WriteReg(LMX2592, 0x1E, 0x0034, 3);
	SPI_WriteReg(LMX2592, 0x1D, 0X0084, 3);
	SPI_WriteReg(LMX2592, 0x1C, 0X2924, 3);
	SPI_WriteReg(LMX2592, 0x19, 0x0000, 3);
	SPI_WriteReg(LMX2592, 0x18, 0X0509, 3);
	SPI_WriteReg(LMX2592, 0x17, 0X8842, 3);
	SPI_WriteReg(LMX2592, 0x16, 0X2300, 3);
	SPI_WriteReg(LMX2592, 0x14, 0X012C, 3);
	SPI_WriteReg(LMX2592, 0x13, 0X0965, 3);
	SPI_WriteReg(LMX2592, 0x0E, 0X018C, 3);
	SPI_WriteReg(LMX2592, 0x0D, 0X4000, 3);
	SPI_WriteReg(LMX2592, 0x0C, 0X7001, 3);
	SPI_WriteReg(LMX2592, 0x0B, 0X0018, 3);
	SPI_WriteReg(LMX2592, 0x0A, 0X10D8, 3);
	SPI_WriteReg(LMX2592, 0x09, 0X0302, 3);
	SPI_WriteReg(LMX2592, 0x08, 0X1084, 3);
	SPI_WriteReg(LMX2592, 0x07, 0X28B2, 3);
	SPI_WriteReg(LMX2592, 0x04, 0X1943, 3);
	SPI_WriteReg(LMX2592, 0x02, 0X0500, 3);
	SPI_WriteReg(LMX2592, 0x01, 0X0808, 3);
	SPI_WriteReg(LMX2592, 0x00, 0X221D, 3);		//DPU Version


	printf("LMX2592 Init Done.\n");
}


void SetRcfmStatBitFreq(uint64_t Freq){
	uint8_t		Segment[3] = {0,0,0};
	uint8_t		SegValue[3] = {0,0,0};
	uint8_t		SegEN[3] = {0,0,0};
	uint8_t		Division = 0;
	uint16_t	SendBuf = 0;
	uint32_t	Int_Value = 0;
	uint16_t	Frac_Value = 0;
	uint8_t 	Prescaler = 2;
	uint32_t	RefCLK = 50000000;
	uint64_t	TargetFreq = 0;
	double		VCOCLK = 0;
	typTableLMX2582 ParamLMX2582 = {0,};


	rcfm_status.rcfm_cal_freq = Freq;
	TargetFreq = Freq;

	if((TargetFreq >= MIN_Freq) && (TargetFreq <= MAX_Freq)){
		ParamLMX2582 = GetPLLValue(TargetFreq);

		//LMX2582 Table Value
		Segment[0]  = ParamLMX2582.u8Segment1;
		Segment[1]  = ParamLMX2582.u8Segment2;
		Segment[2]  = ParamLMX2582.u8Segment3;
		SegValue[0] = ParamLMX2582.u8SegValue1;
		SegValue[1] = ParamLMX2582.u8SegValue2;
		SegValue[2] = ParamLMX2582.u8SegValue3;
		SegEN[0] 	= ParamLMX2582.u8SegEN1;
		SegEN[1] 	= ParamLMX2582.u8SegEN2;
		SegEN[2] 	= ParamLMX2582.u8SegEN3;


		Division = (Segment[0] * Segment[1] * Segment[2]);
		VCOCLK = ((double)(TargetFreq * (uint64_t)Division) / RefCLK);
		SendBuf = (0x0019U |(((uint16_t)SegValue[1] & 0x0FU) << 9)|(((uint16_t)SegEN[2] & 0x01U) << 8)|(((uint16_t)SegEN[1] & 0x01U) << 7)|(((uint16_t)SegValue[0] & 0x01U) << 2)|(((uint16_t)SegEN[0] & 0x01U) << 1));
		SPI_WriteReg(LMX2592, 0x23, SendBuf, 3);

		Int_Value  = (uint32_t)(VCOCLK / Prescaler);
		Frac_Value = (uint16_t)(((VCOCLK / Prescaler)*1000U) - (Int_Value * 1000U));

		SendBuf = ((Int_Value & 0xFFFFU) * Prescaler);
		SPI_WriteReg(LMX2592, 0x26, SendBuf, 3);
		SendBuf = Frac_Value & 0xFFFFU;
		SPI_WriteReg(LMX2592, 0x2D, SendBuf, 3);

		if(TargetFreq < LAST_Freq) {
			SPI_WriteReg(LMX2592, 0x2F, 0x00CF, 3);
			SPI_WriteReg(LMX2592, 0x24, 0x0421, 3);
			SPI_WriteReg(LMX2592, 0x1F, 0x0601, 3);
		}
		else if(TargetFreq >= LAST_Freq) {
			SPI_WriteReg(LMX2592, 0x2F, 0x08CF, 3);
			SPI_WriteReg(LMX2592, 0x24, 0x0011, 3);
			SPI_WriteReg(LMX2592, 0x1F, 0x0401, 3);
		}
		else { }

		SPI_WriteReg(LMX2592, 0x00, 0X221C, 3);
	}

	else{
		printf("Frequency setting is out of range.\n");
	}
}


static typTableLMX2582 GetPLLValue(uint64_t TargetFreq){
	uint32_t MinIndex = 0;
	uint32_t MaxIndex = 0;
	int32_t AvgIndex = 0;
	int32_t OldAvgIndex = -1;
	typTableLMX2582 ParamLMX2582 = {0,};

	MaxIndex = PLL_TABLE_SIZE;
	AvgIndex = ((int32_t)MinIndex + (int32_t)MaxIndex) / (int32_t)2;
	OldAvgIndex = -1;

	if (TargetFreq == TableLMX2582[MaxIndex-(uint32_t)1].u64StartFreq)
	{
		AvgIndex = (int32_t)MaxIndex;
		memcpy(&ParamLMX2582, &TableLMX2582[AvgIndex], sizeof(typTableLMX2582));
		return ParamLMX2582;
	}

	while(1)
	{
		if((TargetFreq >= TableLMX2582[AvgIndex].u64StartFreq) && (TargetFreq < TableLMX2582[AvgIndex + 1].u64StartFreq))
		{
			memcpy(&ParamLMX2582, &TableLMX2582[AvgIndex], sizeof(typTableLMX2582));
			return ParamLMX2582;
		}

		if(TargetFreq > TableLMX2582[AvgIndex].u64StartFreq)
		{
			MinIndex = (uint32_t)AvgIndex;
			AvgIndex = ((int32_t)MinIndex + (int32_t)MaxIndex) / (int32_t)2;
		}
		else
		{
			MaxIndex = (uint32_t)AvgIndex;
			AvgIndex = ((int32_t)MinIndex + (int32_t)MaxIndex) / (int32_t)2;
		}

		if(AvgIndex == OldAvgIndex)
		{
			break;
		}
		else {
			OldAvgIndex = AvgIndex;
		}
	}
	return ParamLMX2582;
}



typTableLMX2582 TableLMX2582[16] =
{
	//	StartFreq			SEG1		SEG2		SEG3		VALUE1		VALUE2		VALUE3		EN1			EN2			EN3			SEL
		{ 20000000,			3,			8,			8,			1,			8,			8,			1,			1,			1,			4},
		{ 28000000,			2,			8,			8,			0,			8,			8,			1,			1,			1,			4},
		{ 37000000,			2,			8,			6,			0,			8,			4,			1,			1,			1,			4},
		{ 56000000,			2,			8,			4,			0,			8,			2,			1,			1,			1,			4},
		{ 74000000,			3,			8,			2,			1,			8,			1,			1,			1,			1,			4},
		{ 99000000,			3,			6,			2,			1,			4,			1,			1,			1,			1,			4},
		{ 111000000,		2,			8,			2,			0,			8,			1,			1,			1,			1,			4},
		{ 148000000,		3,			8,			1,			1,			8,			1,			1,			1,			1,			2},
		{ 222000000,		2,			8,			1,			0,			8,			1,			1,			1,			1,			2},
		{ 296000000,		2,			6,			1,			0,			4,			1,			1,			1,			1,			2},
		{ 444000000,		2,			4,			1,			0,			2,			1,			1,			1,			1,			2},
		{ 592000000,		3,			2,			1,			1,			1,			1,			1,			1,			1,			2},
		{ 888000000,		2,			2,			1,			0,			1,			1,			1,			1,			1,			2},
		{ 1184000000,		3,			1,			1,			1,			1,			1,			1,			1,			1,			1},
		{ 1775000000,		2,			1,			1,			0,			1,			1,			1,			1,			1,			1},
		{ 3550000000,		1,			1,			1,			1,			1,			0,			0,			0,			0,			1}
};
