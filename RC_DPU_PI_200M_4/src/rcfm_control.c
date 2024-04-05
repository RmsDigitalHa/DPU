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
#include <stdbool.h>
#include "xparameters.h"
#include "xgpio.h"
#include "xgpiops.h"
#include "sleep.h"
#include "rcfm_control.h"
#include "Init.h"
#include "rf_control.h"


static XGpio gpio_rcfm_ctrl;

typTableLMX2582 TableLMX2582[16] =
{
	//	StartFreq			SEG1		SEG2		SEG3		VALUE1		VALUE2		VALUE3		EN1			EN2			EN3			SEL
		{ 20000000U,		3U,			8U,			8U,			1U,			8U,			8U,			1U,			1U,			1U,			4U},
		{ 28000000U,		2U,			8U,			8U,			0U,			8U,			8U,			1U,			1U,			1U,			4U},
		{ 37000000U,		2U,			8U,			6U,			0U,			8U,			4U,			1U,			1U,			1U,			4U},
		{ 56000000U,		2U,			8U,			4U,			0U,			8U,			2U,			1U,			1U,			1U,			4U},
		{ 74000000U,		3U,			8U,			2U,			1U,			8U,			1U,			1U,			1U,			1U,			4U},
		{ 99000000U,		3U,			6U,			2U,			1U,			4U,			1U,			1U,			1U,			1U,			4U},
		{ 111000000U,		2U,			8U,			2U,			0U,			8U,			1U,			1U,			1U,			1U,			4U},
		{ 148000000U,		3U,			8U,			1U,			1U,			8U,			1U,			1U,			1U,			1U,			2U},
		{ 222000000U,		2U,			8U,			1U,			0U,			8U,			1U,			1U,			1U,			1U,			2U},
		{ 296000000U,		2U,			6U,			1U,			0U,			4U,			1U,			1U,			1U,			1U,			2U},
		{ 444000000U,		2U,			4U,			1U,			0U,			2U,			1U,			1U,			1U,			1U,			2U},
		{ 592000000U,		3U,			2U,			1U,			1U,			1U,			1U,			1U,			1U,			1U,			2U},
		{ 888000000U,		2U,			2U,			1U,			0U,			1U,			1U,			1U,			1U,			1U,			2U},
		{ 1184000000U,		3U,			1U,			1U,			1U,			1U,			1U,			1U,			1U,			1U,			1U},
		{ 1775000000U,		2U,			1U,			1U,			0U,			1U,			1U,			1U,			1U,			1U,			1U},
		{ 3550000000U,		1U,			1U,			1U,			1U,			1U,			0U,			0U,			0U,			0U,			1U}
};


RCFM_MODE rcfm_status;

/* LNA 1 CTRL(RCFM BD)
 * LNA 1 ON	 - AMP2 Mode
 * LNA 1 OFF - AMP1 Mode
 */
void SetRcfmStatAmpFst(const uint8_t rcfm_amp_fst)
{
	uint32_t Old_Data = 0;
	uint32_t New_Data = 0;

	if(rcfm_amp_fst == RCFM_BYPASS) {
		rcfm_status.rcfm_amp_mode2 = 0x01U;
		rcfm_status.rcfm_amp_mode1 = 0x00U;
	}
	else if(rcfm_amp_fst == RCFM_LNA) {
		rcfm_status.rcfm_amp_mode2 = 0x00U;
		rcfm_status.rcfm_amp_mode1 = 0x01U;
	}
	else { }

	Old_Data = XGpio_DiscreteRead(&RF_GPIO, RF_GPIO_OUT);
	New_Data = (Old_Data & 0xFFFFFFF3U) | (((uint32_t)rcfm_status.rcfm_amp_mode2 & 0x1U) << 3) | (((uint32_t)rcfm_status.rcfm_amp_mode1 & 0x1U) << 2);
	XGpio_DiscreteWrite(&RF_GPIO, RF_GPIO_OUT, New_Data);
}


/*
 * RCFM_Path(SW_V1, BIT_V1)
 */
void SetRcfmStatPath(const uint8_t rcfm_path)
{
	uint32_t Old_Data = 0;
	uint32_t New_Data = 0;

	if(rcfm_path == RCFM_ANT_PATH) {
		rcfm_status.rcfm_rf_select = 0x00U;
	}
	else if(rcfm_path == RCFM_BIT_PATH) {
		rcfm_status.rcfm_rf_select = 0x01U;
	}
	else { }

	Old_Data = XGpio_DiscreteRead(&RF_GPIO, RF_GPIO_OUT);
	New_Data = (Old_Data & 0xFFFFFFEFU) | (((~((uint32_t)rcfm_status.rcfm_rf_select)) & 0x1U) << 4);
	XGpio_DiscreteWrite(&RF_GPIO, RF_GPIO_OUT, New_Data);
}


void SetRcfmStatBitEn(const uint8_t rcfm_bit_en)
{
	uint32_t Old_Data = 0;
	uint32_t New_Data = 0;

	if(rcfm_bit_en == RCFM_CAL_DIS) {
		rcfm_status.rcfm_cal_en = 0x00U;
	}
	else if(rcfm_bit_en == RCFM_CAL_EN) {
		rcfm_status.rcfm_cal_en = 0x01U;
	}
	else { }

	Old_Data = XGpio_DiscreteRead(&RF_GPIO, RF_GPIO_OUT);
	New_Data = (Old_Data & 0xFFFFFFBFU) | (((uint32_t)rcfm_status.rcfm_cal_en & 0x1U) << 6);
	XGpio_DiscreteWrite(&RF_GPIO, RF_GPIO_OUT, New_Data);
}


void SetRcfmStatPathANT(const uint8_t rcfm_path_lna)
{
	uint32_t Old_Data = 0;
	uint32_t New_Data = 0;

	if(rcfm_path_lna == RCFM_ANT_BIAS_OFF) {
		rcfm_status.rcfm_ant_bias = 0x00U;
	}
	else if(rcfm_path_lna == RCFM_ANT_BIAS_ON) {
		rcfm_status.rcfm_ant_bias = 0x01U;
	}
	else { }

	Old_Data = XGpio_DiscreteRead(&RF_GPIO, RF_GPIO_OUT);
	New_Data = (Old_Data & 0xFFFFFFDFU) | (((uint32_t)rcfm_status.rcfm_ant_bias & 0x1U) << 5);
	XGpio_DiscreteWrite(&RF_GPIO, RF_GPIO_OUT, New_Data);
}


void Init_BIT_PLL(void){
	//SPI Default Value Setting
	SPI_WriteReg(LMX2592, 0x46U, 0x0000U, 3U);
	SPI_WriteReg(LMX2592, 0x45U, 0x0000U, 3U);
	SPI_WriteReg(LMX2592, 0x44U, 0x0089U, 3U);
	SPI_WriteReg(LMX2592, 0x40U, 0x0077U, 3U);
	SPI_WriteReg(LMX2592, 0x3EU, 0x0000U, 3U);
	SPI_WriteReg(LMX2592, 0x3DU, 0x0001U, 3U);
	SPI_WriteReg(LMX2592, 0x3BU, 0x0000U, 3U);

	SPI_WriteReg(LMX2592, 0x30U, 0X03FCU, 3U);
	SPI_WriteReg(LMX2592, 0x2FU, 0X00CFU, 3U);
	SPI_WriteReg(LMX2592, 0x2EU, 0X0FA3U, 3U);
	SPI_WriteReg(LMX2592, 0x2DU, 0x0000U, 3U);
	SPI_WriteReg(LMX2592, 0x2CU, 0x0000U, 3U);
	SPI_WriteReg(LMX2592, 0x2BU, 0x0000U, 3U);
	SPI_WriteReg(LMX2592, 0x2AU, 0x0000U, 3U);
	SPI_WriteReg(LMX2592, 0x29U, 0X03E8U, 3U);
	SPI_WriteReg(LMX2592, 0x28U, 0x0000U, 3U);
	SPI_WriteReg(LMX2592, 0x27U, 0X8204U, 3U);
	SPI_WriteReg(LMX2592, 0x26U, 0X0050U, 3U);		//DPU Version
	SPI_WriteReg(LMX2592, 0x25U, 0X4000U, 3U);
	SPI_WriteReg(LMX2592, 0x24U, 0X0421U, 3U);
	SPI_WriteReg(LMX2592, 0x23U, 0X049BU, 3U);
	SPI_WriteReg(LMX2592, 0x22U, 0XC3EAU, 3U);
	SPI_WriteReg(LMX2592, 0x21U, 0X2A0AU, 3U);
	SPI_WriteReg(LMX2592, 0x20U, 0X210AU, 3U);
	SPI_WriteReg(LMX2592, 0x1FU, 0X0601U, 3U);

	SPI_WriteReg(LMX2592, 0x1EU, 0x0034U, 3U);
	SPI_WriteReg(LMX2592, 0x1DU, 0X0084U, 3U);
	SPI_WriteReg(LMX2592, 0x1CU, 0X2924U, 3U);
	SPI_WriteReg(LMX2592, 0x19U, 0x0000U, 3U);
	SPI_WriteReg(LMX2592, 0x18U, 0X0509U, 3U);
	SPI_WriteReg(LMX2592, 0x17U, 0X8842U, 3U);
	SPI_WriteReg(LMX2592, 0x16U, 0X2300U, 3U);
	SPI_WriteReg(LMX2592, 0x14U, 0X012CU, 3U);
	SPI_WriteReg(LMX2592, 0x13U, 0X0965U, 3U);
	SPI_WriteReg(LMX2592, 0x0EU, 0X018CU, 3U);
	SPI_WriteReg(LMX2592, 0x0DU, 0X4000U, 3U);
	SPI_WriteReg(LMX2592, 0x0CU, 0X7001U, 3U);
	SPI_WriteReg(LMX2592, 0x0BU, 0X0018U, 3U);
	SPI_WriteReg(LMX2592, 0x0AU, 0X10D8U, 3U);
	SPI_WriteReg(LMX2592, 0x09U, 0X0302U, 3U);
	SPI_WriteReg(LMX2592, 0x08U, 0X1084U, 3U);
	SPI_WriteReg(LMX2592, 0x07U, 0X28B2U, 3U);
	SPI_WriteReg(LMX2592, 0x04U, 0X1943U, 3U);
	SPI_WriteReg(LMX2592, 0x02U, 0X0500U, 3U);
	SPI_WriteReg(LMX2592, 0x01U, 0X0808U, 3U);
	SPI_WriteReg(LMX2592, 0x00U, 0X221DU, 3U);		//DPU Version


	printf("LMX2592 Init Done.\n");
}


void SetRcfmStatBitFreq(const uint64_t Freq){
	uint8_t		Segment[3] = {0,0,0};
	uint8_t		SegValue[3] = {0,0,0};
	uint8_t		SegEN[3] = {0,0,0};
	uint8_t		Division = 0;
	uint16_t	SendBuf = 0;
	uint32_t	Int_Value = 0;
	uint16_t	Frac_Value = 0;
	const uint8_t 	Prescaler = 2U;
	const uint32_t	RefCLK = 50000000U;
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
		SPI_WriteReg(LMX2592, 0x23U, SendBuf, 3U);

		Int_Value  = (uint32_t)(VCOCLK / Prescaler);
		Frac_Value = (uint16_t)(((VCOCLK / Prescaler)*1000U) - (Int_Value * 1000U));


		SendBuf = ((Int_Value & 0xFFFFU) * Prescaler);
		SPI_WriteReg(LMX2592, 0x26U, SendBuf, 3U);
		SendBuf = Frac_Value & 0xFFFFU;
		SPI_WriteReg(LMX2592, 0x2DU, SendBuf, 3U);

		if(TargetFreq < LAST_Freq) {
			SPI_WriteReg(LMX2592, 0x2FU, 0x00CFU, 3U);
			SPI_WriteReg(LMX2592, 0x24U, 0x0421U, 3U);
			SPI_WriteReg(LMX2592, 0x1FU, 0x0601U, 3U);
		}
		else if(TargetFreq >= LAST_Freq) {
			SPI_WriteReg(LMX2592, 0x2FU, 0x08CFU, 3U);
			SPI_WriteReg(LMX2592, 0x24U, 0x0011U, 3U);
			SPI_WriteReg(LMX2592, 0x1FU, 0x0401U, 3U);
		}
		else { }

		SPI_WriteReg(LMX2592, 0x00U, 0X221CU, 3U);
	}

	else{
		printf("Frequency setting is out of range.\n");
	}
}


static typTableLMX2582 GetPLLValue(const uint64_t TargetFreq){
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
		(void)memcpy(&ParamLMX2582, &TableLMX2582[AvgIndex], sizeof(typTableLMX2582));
		return ParamLMX2582;
	}

	while(true)
	{
		if((TargetFreq >= TableLMX2582[AvgIndex].u64StartFreq) && (TargetFreq < TableLMX2582[AvgIndex + 1].u64StartFreq))
		{
			(void)memcpy(&ParamLMX2582, &TableLMX2582[AvgIndex], sizeof(typTableLMX2582));
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
