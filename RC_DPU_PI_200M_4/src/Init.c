/*
 * Init.c
 *
 *  Created on: 2023. 12. 27.
 *      Author: RMS_Digital
 */

//header
#include "xil_io.h"
#include "xparameters.h"
#include "xspips.h"
#include "xgpio.h"
#include "xiicps.h"
#include "sleep.h"

#include "rts_spectrum_ctrl.h"
#include "axi_rc_spectrum.h"
#include "adrv9008.h"
#include "Init.h"
#include "rf_control.h"
#include "icd_parser.h"
#include "rcfm_control.h"
#include "rcrm_control.h"
#include "user_func.h"


static uint8_t		u8SpiData_RF[4] = {0,0,0,0};
static uint8_t		u8SpiData_LOG[3] = {0,0,0};
static uint8_t		u32Data_RF[4] 	= {0,0,0,0};
static uint8_t		u8Data_LOG[2] 	= {0,0};

uint32_t dpu_iter_count = 1U;
uint32_t dpu_ref_level = 0x80000000U;
uint32_t dpu_win_func = 1U;

static uint8_t SendBuffer[I2C_BUFFER_SIZE];    /**< Buffer for Transmitting Data */
static uint8_t RecvBuffer[I2C_BUFFER_SIZE];    /**< Buffer for Receiving Data */

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
int Init_FPGA_CTRL(void){
	int Status;

	Status = Init_GPIO_CTRL();
	if(Status != XST_SUCCESS){
		printf("GPIO Initialize Fail.\n");
		return XST_FAILURE;
	}

	Status = Init_SPI_CTRL();
	if(Status != XST_SUCCESS){
		printf("SPI Initialize Fail.\n");
		return XST_FAILURE;
	}

	Status = Init_I2C_CTRL();
	if(Status != XST_SUCCESS){
		printf("I2C Initialize Fail.\n");
		return XST_FAILURE;
	}

	RTS_SPECTRUM_CTRL_mWriteReg(RC_SPCTRUM_BaseAddr, REG_RTS_FFT_SCALE, 0x1AAB);	//0x2AB	0x6AB
	RTS_SPECTRUM_CTRL_mWriteReg(RC_SPCTRUM_BaseAddr, REG_RTS_FFT_nFFT, 0xD);
	RTS_SPECTRUM_CTRL_mWriteReg(RC_SPCTRUM_BaseAddr, REG_FFT_SIZE, 0x2);
	RTS_SPECTRUM_CTRL_mWriteReg(RC_SPCTRUM_BaseAddr, REG_RTS_DMA_START, 0x1);
	RTS_SPECTRUM_CTRL_mWriteReg(RC_SPCTRUM_BaseAddr, REG_MULT_PARM, 0x7);
	(void)rts_start(RC_SPCTRUM_BaseAddr, dpu_iter_count, dpu_ref_level, dpu_win_func);
	(void)rts_end(RC_SPCTRUM_BaseAddr);
	(void)rts_start(RC_SPCTRUM_BaseAddr, dpu_iter_count, dpu_ref_level, dpu_win_func);

	printf("FPGA Initialize Success.\n");
	return XST_SUCCESS;
}


int Init_RF_CTRL(void){
	uint32_t Old_Data = 0;
	uint32_t New_Data = 0;

	//RF Switch ON
	Old_Data = XGpio_DiscreteRead(&RF_GPIO, RF_GPIO_OUT);
	New_Data = (Old_Data & 0xFFFFFFFCU) | (0x03U);
	XGpio_DiscreteWrite(&RF_GPIO, RF_GPIO_OUT, New_Data);

	SetRcfmStatBitEn(RCFM_CAL_DIS);
	SetRcrmStatFreq(INIT_FREQ + FREQ_OFFSET);
	SetRcfmStatPath(RCFM_ANT_PATH);
	SetRcfmStatAmpFst(RCFM_BYPASS);
	SetRcrmStatAmpScd(RCRM_BYPASS02);
	SetRcrmStatSysAtt(0);
	SetRcrmStatGainAtt(0);
	xil_printf("\n0. Default Bit Frequency : 1000MHz");
	SetRcfmStatBitFreq(INIT_FREQ);
	SetRcfmStatPathANT(RCFM_ANT_BIAS_OFF);

	RF_STATUS.RCFM_LNA_MODE = RCFM_BYPASS;
	RF_STATUS.RCRM_LNA_MODE = RCRM_BYPASS02;
	RF_STATUS.RCV_PATH = RCFM_ANT_PATH;
	DPU_STATUS.CenterFreq = INIT_FREQ;
	RF_STATUS.RF_BIT_EN = RCFM_CAL_DIS;
	RF_STATUS.ANT_PATH = RCFM_ANT_BIAS_OFF;

	GetStatusPBIT();

	printf("RF Initialize Success.\n");
	return XST_SUCCESS;
}


static int Init_GPIO_CTRL(void){
	int status;

	status = XGpio_Initialize(&RF_GPIO, GPIO_RF_CTRL_DEVICE_ID);
	if(status != XST_SUCCESS){
		printf("GPIO Initialize Fail.\n");
		return XST_FAILURE;
	}

	XGpio_SetDataDirection(&RF_GPIO, RF_GPIO_IN, GPIO_IN);
	XGpio_SetDataDirection(&RF_GPIO, RF_GPIO_OUT, GPIO_OUT);


	status = XGpio_Initialize(&PWR_GPIO, GPIO_BD_PWR_DEVICE_ID);
	if(status != XST_SUCCESS){
		printf("GPIO Initialize Fail.\n");
		return XST_FAILURE;
	}
	XGpio_SetDataDirection(&PWR_GPIO, PWR_GPIO_IN, GPIO_IN);

	return 0;
}


static int Init_SPI_CTRL(void){
	int Status;
	XSpiPs_Config *SpiConfig;

	/*
	 * Initialize the SPI driver so that it's ready to use
	 */
	SpiConfig = XSpiPs_LookupConfig(SPI_RF_CTRL);
	if (NULL == SpiConfig) {
		printf("SPI Initialize Fail.\n");
		return XST_FAILURE;
	}

	Status = XSpiPs_CfgInitialize(&SPI_RF, SpiConfig,
				       SpiConfig->BaseAddress);
	if (Status != XST_SUCCESS) {
		printf("SPI Initialize Fail.\n");
		return XST_FAILURE;
	}

	XSpiPs_SetOptions(&SPI_RF, XSPIPS_MASTER_OPTION | XSPIPS_FORCE_SSELECT_OPTION);

	XSpiPs_SetClkPrescaler(&SPI_RF, XSPIPS_CLK_PRESCALE_16);

	return 0;
}


static int Init_I2C_CTRL(void){
	int Status;
	XIicPs_Config *I2C_0_Config;
	XIicPs_Config *I2C_1_Config;

	I2C_0_Config = XIicPs_LookupConfig(I2C_Driver_0);
	if (NULL == I2C_0_Config) {
		return XST_FAILURE;
	}

	Status = XIicPs_CfgInitialize(&I2C_0, I2C_0_Config, I2C_0_Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XIicPs_SetSClk(&I2C_0, IIC_SCLK_RATE);


	I2C_1_Config = XIicPs_LookupConfig(I2C_Driver_1);
	if (NULL == I2C_1_Config) {
		return XST_FAILURE;
	}

	Status = XIicPs_CfgInitialize(&I2C_1, I2C_1_Config, I2C_1_Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XIicPs_SetSClk(&I2C_1, IIC_SCLK_RATE);

	return 0;
}

int8_t GetRFTmp(const uint8_t dev){
	int Status;
	if(dev == TMP_RCFM_DEV){
		//RCFM TMP
		Status = XIicPs_MasterSendPolled(&I2C_0, SendBuffer, I2C_BUFFER_SIZE, TMP_RCFM_ADDR);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		//Wait until bus is idle to start another transfer.
		while (XIicPs_BusIsBusy(&I2C_0)) {
			/* NOP */
		}

		Status = XIicPs_MasterRecvPolled(&I2C_0, RecvBuffer, I2C_BUFFER_SIZE, TMP_RCFM_ADDR);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		if(((RecvBuffer[0] >> 7U) & 0x01U) == 0x01U){
			return (int8_t)(RecvBuffer[0] - 256U);
		}
		else{
			return (int8_t)RecvBuffer[0];
		}
	}

	else{
		//RCRM TMP
		Status = XIicPs_MasterSendPolled(&I2C_0, SendBuffer, I2C_BUFFER_SIZE, TMP_RCRM_ADDR);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		//Wait until bus is idle to start another transfer.
		while (XIicPs_BusIsBusy(&I2C_0)) {
			/* NOP */
		}

		Status = XIicPs_MasterRecvPolled(&I2C_0, RecvBuffer, I2C_BUFFER_SIZE, TMP_RCRM_ADDR);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		if(((RecvBuffer[0] >> 7U) & 0x01U) == 0x01U){
			return (RecvBuffer[0] - 256U);
		}
		else{
			return RecvBuffer[0];
		}

	}

}


void SPI_WriteReg(const uint8_t dev, const uint16_t Addr, const uint32_t val, const uint8_t NumByte){
	uint64_t	Buffer = 0;
	uint8_t		Addr_LMX = 0;
	uint64_t	Send_Buf = 0;

	switch(dev){
	case LMX2592 :
		if(NumByte == 3U){
			Addr_LMX = (Addr & (0xFFU));
			Send_Buf = ((uint64_t)val & 0x0000FFFFU);

			Buffer = (((uint64_t)Addr_LMX << 16) | Send_Buf);

			u8SpiData_RF[0] = ((Buffer & (0xFF0000U))>>16U);
			u8SpiData_RF[1] = ((Buffer & (0x00FF00U))>>8U);
			u8SpiData_RF[2] =  (Buffer & (0x0000FFU));
		}
		else{}

		XSpiPs_SetOptions(&SPI_RF, XSPIPS_MASTER_OPTION | XSPIPS_FORCE_SSELECT_OPTION);
		XSpiPs_SetSlaveSelect(&SPI_RF, 1);
		XSpiPs_PolledTransfer(&SPI_RF, u8SpiData_RF, NULL, NumByte);		// 2=> 16bit (8x2)

		usleep(1000);
		break;
	case RF_CTRL :
		if(NumByte == 4U){
			Send_Buf = val;

			u8SpiData_RF[0] = ((Send_Buf & (0xFF000000U))>>24);
			u8SpiData_RF[1] = ((Send_Buf & (0x00FF0000U))>>16);
			u8SpiData_RF[2] = ((Send_Buf & (0x0000FF00U))>>8);
			u8SpiData_RF[3] = (Send_Buf & (0x000000FFU));
		}
		else{}

		XSpiPs_SetSlaveSelect(&SPI_RF, 0);
		XSpiPs_PolledTransfer(&SPI_RF, u8SpiData_RF, NULL, NumByte);		// 2=> 16bit (8x2)

		usleep(10);
		break;
	case DPU_LOG :
		break;
	default	:	break;
	}

}

//Function type casting
uint16_t SPI_ReadReg(const uint8_t dev, const uint8_t Addr, const uint8_t NumByte){
	uint32_t	Buffer = 0;
	uint16_t	Addr_RF = 0;
	double		Log_Step = 9.765625;
	uint16_t	TMP_Value = 0;
	uint16_t	Log_Value = 0;
	uint16_t	Data_Return = 0;

	switch(dev){
	case LMX2592 :
		Addr_RF = ((uint16_t)Addr & 0xFFU);
		Buffer = (((uint32_t)Addr_RF << 16) | ((uint32_t)1U << 23));

		u8SpiData_RF[0] = ((Buffer & (0xFF0000U))>>16);
		u8SpiData_RF[1] = 0;
		u8SpiData_RF[2] = 0;

		XSpiPs_SetSlaveSelect(&SPI_RF, 1);
		XSpiPs_PolledTransfer(&SPI_RF, u8SpiData_RF, u32Data_RF, NumByte);		// 2=> 16bit (8x2)
		usleep(time_20us);
		Data_Return = u32Data_RF;
		break;
	case RF_CTRL :			//Unable to read
		printf("This Function is not used.\n");
		Data_Return = 0U;
		break;
	case DPU_LOG :
		XSpiPs_SetOptions(&SPI_RF, XSPIPS_MASTER_OPTION | XSPIPS_FORCE_SSELECT_OPTION | XSPIPS_CLK_ACTIVE_LOW_OPTION);

		u8SpiData_LOG[0] = 0x83U;
		u8SpiData_LOG[1] = 0x30U;

		XSpiPs_SetSlaveSelect(&SPI_RF, 2);
		XSpiPs_PolledTransfer(&SPI_RF, u8SpiData_LOG, u8Data_LOG, NumByte);		// 2=> 16bit (8x2)
		TMP_Value = (((uint16_t)u8Data_LOG[0] & (uint16_t)0x0FU) << 4) | (((uint16_t)u8Data_LOG[1] & (uint16_t)0xF0U) >> 4);
		Log_Value = (uint16_t)(TMP_Value * Log_Step);

		XSpiPs_SetSlaveSelect(&SPI_RF, 2);
		XSpiPs_PolledTransfer(&SPI_RF, u8SpiData_LOG, u8Data_LOG, NumByte);		// 2=> 16bit (8x2)
		TMP_Value = (((uint16_t)u8Data_LOG[0] & (uint16_t)0x0FU) << 4) | (((uint16_t)u8Data_LOG[1] & (uint16_t)0xF0U) >> 4);

		Log_Value = (uint16_t)(TMP_Value * Log_Step);

		Data_Return = Log_Value;
		break;
	default :
		break;
	}

	return Data_Return;
}



