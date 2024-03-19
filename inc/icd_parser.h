/* icd_parser.h
 *
 *  Created on: 2022. 11. 21.
 *      Author: hadud
 */
#include <stdint.h>

#ifndef SRC_APP_TCP_SERVER_ICD_PARSER_H_
#define SRC_APP_TCP_SERVER_ICD_PARSER_H_

#define ICD_HEADER_SIZE		8
#define SPEC_HEADER_SIZE	12


//DPU CTRL
#define SET_CENTER_FREQ	 	0xE1510010
#define SET_BW		  		0xE1510020
#define SET_RBW		  		0xE1510030
#define SET_SPEC_START 		0xE1510040
#define SET_SPEC_STOP 		0xE1510050
#define SET_REQUEST_BIT	 	0xE1510060

#define TEST_ITER_CNT	 	0xE1510070
#define TEST_REF_LEVEL		0xE1510080
#define TEST_ADRV_GAIN		0xE1510090

//RF CTRL
#define SET_RCV_FREQ	 	0xE1510100
#define SET_FILTER_PATH	 	0xE1510110
#define SET_AMP_MODE	 	0xE1510120
#define SET_RCV_ATTEN	 	0xE1510130
#define SET_RCV_PATH	 	0xE1510140
#define SET_BIT_CTRL	 	0xE1510150
#define SET_ANT_PATH	 	0xE1510160
#define GET_RF_STATUS	 	0xE1510170
#define GET_RF_LOG	 		0xE1510180		//RCRM LOG
#define GET_RF_TMP		 	0xE1510190
#define SET_LNA_MODE	 	0xE1510200
#define SET_JAM_START	 	0xE1510210		//JAM Mode Start
#define SET_JAM_STOP	 	0xE1510220		//RCV Mode Start

#define BW_10M			10000000
#define BW_30M			30000000
#define BW_50M			50000000
#define BW_100M			96000000
#define BW_200M			200000000
#define BW_500M			500000000
#define BW_ALL			5616000000

#define RBW_8kHz		8000
#define RBW_15kHz		15000
#define RBW_30kHz		30000
#define RBW_60kHz		60000
#define RBW_120kHz		120000
#define RBW_240kHz		240000


#define FFT_256_BIN		420
#define FFT_512_BIN		836
#define FFT_1024_BIN	1668
#define FFT_2048_BIN	3336
#define FFT_4098_BIN	6668
#define FFT_8192_BIN	13336


//RF_CTRL
#define SET		0
#define GET		1

#define RCFM	0
#define RCRM	1

#define AMP_BYPASS	0
#define AMP_MODE_1	1
#define AMP_MODE_2	2

#define SYS_ATTEN		0
#define GAIN_ATTEN		1

#define PATH_ANT		0
#define PATH_BIT		1


#define CBIT 0x00
#define PBIT 0x01
#define IBIT 0x02

#define JAM_STOP	0
#define JAM_START	1


typedef struct ICD_HEADER{
	uint8_t SRC_CODE;
	uint8_t DEST_CODE;
	uint16_t CMD_CODE;
	uint32_t COMM_SET;
	uint16_t SRC_ID;
	uint16_t DEST_ID;
	uint32_t DATA_SIZE;
}ICD_HEADER;


typedef struct {
	uint64_t 	CenterFreq;
	uint8_t		ParmBw;
	uint8_t		ParmRbw;
	uint16_t	SpecBin;
	uint64_t 	BandWidth;
	uint32_t 	RBW;
	uint8_t 	START;
	uint8_t		FLAG_SET_FREQ;
	uint8_t		FLAG_DATA_SEND;
	uint8_t		ChNum;
	uint8_t		IterCnt;
	uint8_t		ScanMode;
}RECV_SETTING;

typedef struct {
	uint64_t 	RCV_FREQ;
	uint8_t		RCRM_BPF_PATH;
	uint8_t		RCRM_LPF_PATH;
	uint8_t		RCFM_LNA_MODE;		//LNA 1
	uint8_t 	RCRM_LNA_MODE;		//LNA 2
	uint8_t 	ATTEN_SYS;
	uint8_t 	ATTEN_GAIN;
	uint8_t		RCV_PATH;
	uint8_t		RF_BIT_EN;
	uint64_t	RF_BIT_FREQ;
	uint8_t		ANT_PATH;
	uint8_t		LOG_RCRM;
	uint8_t		TMP_RCRM;
	int8_t		TMP_RCFM;
	uint8_t		JAM_RCV_PATH;
	uint8_t		JAM_ANT_PATH;
	uint8_t		JAM_RCFM_LNA_MODE;		//LNA 1
	uint8_t 	JAM_RCRM_LNA_MODE;		//LNA 2
}RF_SETTING;

typedef struct {
	uint8_t 	LOCK_ADCLK;
	uint8_t		LOCK_BIT;
	uint8_t		LNA1;
	uint8_t		REF_SIG;
	uint8_t 	RCFM_PWR;
	uint8_t 	RCFM_INSERT;
	int8_t 		RCFM_TMP;
	uint8_t		LNA2;
	uint8_t		LNA3;
	uint8_t		RCRM_PWR;
	uint8_t		RCRM_INSERT;
	int8_t		RCRM_TMP;
	uint8_t		DONE_FPGA;
	uint8_t		DONE_ADC;
	uint8_t		DONE_DDR;
	uint8_t		RF_PATH;
	uint16_t	RF_LOG;
	uint8_t		BIT_SET;
}HW_CHECK;

extern RECV_SETTING DPU_STATUS;
extern HW_CHECK BIT_STATUS;
extern HW_CHECK PBIT_STATUS;
extern RF_SETTING RF_STATUS;
extern int spec_packet_size;

//////// Function Definitions ////////
ICD_HEADER ParserTCP(uint8_t *recv_buffer, uint16_t packet_len);
void SwapOPCODE(uint8_t* buf);

#endif /* SRC_APP_TCP_SERVER_ICD_PARSER_H_ */

