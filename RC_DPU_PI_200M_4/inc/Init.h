/*
 * Init.h
 *
 *  Created on: 2023. 12. 27.
 *      Author: RMS_Digital
 */

#ifndef INC_INIT_H_
#define INC_INIT_H_

#include "xspips.h"
#include "xgpio.h"
#include "xiicps.h"

#define GPIO_BD_PWR_DEVICE_ID  XPAR_BD_PWR_DEVICE_ID
#define GPIO_RF_CTRL_DEVICE_ID  XPAR_RF_CTRL_DEVICE_ID

#define	LMX2592		0x0
#define	RF_CTRL		0x1
#define DPU_LOG		0x2

#define GPIO_OUT	0
#define GPIO_IN		1

#define RF_GPIO_OUT		1
#define RF_GPIO_IN		2
#define PWR_GPIO_IN		1

#define time_1us	1
#define time_10us	10
#define time_20us	20
#define time_30us	30
#define time_50us	50

#define INIT_FREQ	1000000000

#define TMP_DPU_ADDR		0x1C
#define TMP_RCFM_ADDR		0x4C
#define TMP_RCRM_ADDR		0x4D

#define TMP_RCFM_DEV	0
#define TMP_RCRM_DEV	1

#define I2C_BUFFER_SIZE		1
#define IIC_SCLK_RATE		100000

#define SPI_RF_CTRL		XPAR_XSPIPS_1_DEVICE_ID

#define I2C_Driver_0	XPAR_XIICPS_0_DEVICE_ID
//CLK_IC (SI570)
//CLK_IC (SI5340)
//EEPROM (on DPU)
//RF_BD

#define I2C_Driver_1	XPAR_XIICPS_1_DEVICE_ID
//CLK_IC (570BAB)
//TMP (on DPU)



XGpio RF_GPIO; /* The Instance of the GPIO Driver */
XGpio PWR_GPIO; /* The Instance of the GPIO Driver */
XSpiPs SPI_RF;
XIicPs I2C_0;
XIicPs I2C_1;

int Init_FPGA_CTRL();
int Init_RF_CTRL();
int Init_GPIO_CTRL();
int Init_SPI_CTRL();
int Init_I2C_CTRL();
uint8_t GetRFTmp(uint8_t dev);
int GetDPUTmp();
void SPI_WriteReg(uint8_t dev, uint16_t Addr, uint32_t val, uint8_t NumByte);
uint16_t SPI_ReadReg(uint8_t dev, uint8_t Addr, uint8_t NumByte);


#endif /* INC_INIT_H_ */
