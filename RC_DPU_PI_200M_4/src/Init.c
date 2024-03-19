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

//extern XGpio RF_GPIO; /* The Instance of the GPIO Driver */
//extern XGpio PWR_GPIO; /* The Instance of the GPIO Driver */

static uint8_t		u8SpiData_RF[3] = {0, };
static uint8_t		u8SpiData_LOG[3] = {0, };
static uint8_t		u32Data_RF[4] 	= {0,};
static uint8_t		u8Data_LOG[2] 	= {0,};

uint32_t dpu_iter_count = 1;
uint32_t dpu_ref_level = 0x80000000;
uint32_t dpu_win_func = 1;

extern HW_CHECK BIT_STATUS;
extern RF_SETTING	 RF_STATUS;
extern RECV_SETTING DPU_STATUS;

uint8_t SendBuffer[I2C_BUFFER_SIZE];    /**< Buffer for Transmitting Data */
uint8_t RecvBuffer[I2C_BUFFER_SIZE];    /**< Buffer for Receiving Data */

//XGpio RF_GPIO;		//10bit Input
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

//XGpio PWR_GPIO;		//10bit Input
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
int Init_FPGA_CTRL(){
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
	rts_start(RC_SPCTRUM_BaseAddr, dpu_iter_count, dpu_ref_level, dpu_win_func);
	rts_end(RC_SPCTRUM_BaseAddr);
	rts_start(RC_SPCTRUM_BaseAddr, dpu_iter_count, dpu_ref_level, dpu_win_func);

	printf("FPGA Initialize Success.\n");
	return XST_SUCCESS;
}


int Init_RF_CTRL(){
	uint32_t Old_Data = 0;
	uint32_t New_Data = 0;

	//RF Switch ON
	Old_Data = XGpio_DiscreteRead(&RF_GPIO, RF_GPIO_OUT);
	New_Data = (Old_Data & 0xFFFFFFFC) | (0x03);
	XGpio_DiscreteWrite(&RF_GPIO, RF_GPIO_OUT, New_Data);

	SetRcfmStatBitEn(RCFM_CAL_DIS);
//	SetRcrmStatFreq(INIT_FREQ + FREQ_OFFSET - FREQ_NCO);
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


int Init_GPIO_CTRL(){
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

//	printf("GPIO Initialize Success.\n");
	return 0;
}


int Init_SPI_CTRL(){
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

int Init_SPI_LOG(){
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

	XSpiPs_SetOptions(&SPI_RF, XSPIPS_MASTER_OPTION | XSPIPS_FORCE_SSELECT_OPTION | XSPIPS_CLK_ACTIVE_LOW_OPTION);

	XSpiPs_SetClkPrescaler(&SPI_RF, XSPIPS_CLK_PRESCALE_64);

	return 0;
}

int Init_I2C_CTRL(){
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

//	printf("I2C Initialize Success.\n");
	return 0;
}

uint8_t GetRFTmp(uint8_t dev){
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

		if(((RecvBuffer[0] >> 7) & 0x01) == 0x01){
//			BIT_STATUS.RCFM_TMP = RecvBuffer[0] - 256;
			return (RecvBuffer[0] - 256);
		}
		else{
//			BIT_STATUS.RCFM_TMP = RecvBuffer[0];
			return RecvBuffer[0];
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

		if(((RecvBuffer[0] >> 7) & 0x01) == 0x01){
	//		BIT_STATUS.RCFM_TMP = (int8_t)(RecvBuffer[0] - 256);
			return (RecvBuffer[0] - 256);
		}
		else{
	//		BIT_STATUS.RCRM_TMP = (int8_t)RecvBuffer[0];
			return RecvBuffer[0];
		}

	//	return XST_SUCCESS;
	}

//	usleep(100);
}

int GetDPUTmp(){
	int Status;

	//RCFM TMP
	Status = XIicPs_MasterSendPolled(&I2C_1, SendBuffer, I2C_BUFFER_SIZE, TMP_DPU_ADDR);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	//Wait until bus is idle to start another transfer.
	while (XIicPs_BusIsBusy(&I2C_1)) {
		/* NOP */
	}

	Status = XIicPs_MasterRecvPolled(&I2C_1, RecvBuffer, I2C_BUFFER_SIZE, TMP_DPU_ADDR);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
//	BIT_STATUS.RCFM_TMP = RecvBuffer[0];

	return 0;
}


void SPI_WriteReg(uint8_t dev, uint16_t Addr, uint32_t val, uint8_t NumByte){
	uint64_t	Buffer = 0;
	uint8_t		Addr_LMX = 0;
	uint64_t	Send_Buf = 0;

	switch(dev){
	case LMX2592 :
		if(NumByte == 3){
			Addr_LMX = (Addr & (0xFF));
			Send_Buf = (val & (0x0000FFFF));

			Buffer = ((Addr_LMX << 16) | Send_Buf);

			u8SpiData_RF[0] = ((Buffer & (0xFF0000))>>16);
			u8SpiData_RF[1] = ((Buffer & (0x00FF00))>>8);
			u8SpiData_RF[2] =  (Buffer & (0x0000FF));
		}
		else{}

		XSpiPs_SetOptions(&SPI_RF, XSPIPS_MASTER_OPTION | XSPIPS_FORCE_SSELECT_OPTION);
		XSpiPs_SetSlaveSelect(&SPI_RF, 1);
		XSpiPs_PolledTransfer(&SPI_RF, u8SpiData_RF, NULL, NumByte);		// 2=> 16bit (8x2)

//		return 0;
		usleep(1000);
		break;
	case RF_CTRL :
		if(NumByte == 4){
			Send_Buf = val;

			u8SpiData_RF[0] = ((Send_Buf & (0xFF000000))>>24);
			u8SpiData_RF[1] = ((Send_Buf & (0x00FF0000))>>16);
			u8SpiData_RF[2] = ((Send_Buf & (0x0000FF00))>>8);
			u8SpiData_RF[3] = (Send_Buf & (0x000000FF));
		}
		else{}

		XSpiPs_SetSlaveSelect(&SPI_RF, 0);
		XSpiPs_PolledTransfer(&SPI_RF, u8SpiData_RF, NULL, NumByte);		// 2=> 16bit (8x2)

//		return 0;
		usleep(10);
		break;
	case DPU_LOG :
		break;
	default	:	break;
	}

}

//Function type casting
uint16_t SPI_ReadReg(uint8_t dev, uint8_t Addr, uint8_t NumByte){
	uint32_t	Buffer = 0;
	uint16_t	Addr_RF = 0;
	double		Log_Step = 9.765625;
	uint16_t	TMP_Value = 0;
	uint16_t	Log_Value = 0;

	switch(dev){
	case LMX2592 :
		Addr_RF = (Addr & (0xFF));
		Buffer = ((Addr_RF << 16) | (1 << 23));

		u8SpiData_RF[0] = ((Buffer & (0xFF0000))>>16);
		u8SpiData_RF[1] = 0;
		u8SpiData_RF[2] = 0;

		XSpiPs_SetSlaveSelect(&SPI_RF, 1);
		XSpiPs_PolledTransfer(&SPI_RF, u8SpiData_RF, u32Data_RF, NumByte);		// 2=> 16bit (8x2)
		return u32Data_RF;
		usleep(time_20us);
//		return 0;
		break;
	case RF_CTRL :			//Unable to read

		return 0;
		break;
	case DPU_LOG :
		XSpiPs_SetOptions(&SPI_RF, XSPIPS_MASTER_OPTION | XSPIPS_FORCE_SSELECT_OPTION | XSPIPS_CLK_ACTIVE_LOW_OPTION);

		u8SpiData_LOG[0] = 0x83;
		u8SpiData_LOG[1] = 0x30;

		XSpiPs_SetSlaveSelect(&SPI_RF, 2);
		XSpiPs_PolledTransfer(&SPI_RF, u8SpiData_LOG, u8Data_LOG, NumByte);		// 2=> 16bit (8x2)
		TMP_Value = ((u8Data_LOG[0] & 0x0F) << 4) | ((u8Data_LOG[1] & 0xF0) >> 4);
//		Log_Value = (uint16_t)((TMP_Value * Log_Step) + 0x32);	//offset?
		Log_Value = (uint16_t)(TMP_Value * Log_Step);

		XSpiPs_SetSlaveSelect(&SPI_RF, 2);
		XSpiPs_PolledTransfer(&SPI_RF, u8SpiData_LOG, u8Data_LOG, NumByte);		// 2=> 16bit (8x2)
		TMP_Value = ((u8Data_LOG[0] & 0x0F) << 4) | ((u8Data_LOG[1] & 0xF0) >> 4);
//		Log_Value = (uint16_t)((TMP_Value * Log_Step) + 0x32);	//offset?
		Log_Value = (uint16_t)(TMP_Value * Log_Step);

//		BIT_STATUS.RF_LOG = Log_Value;
		return Log_Value;
		break;
	}
}


