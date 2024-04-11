/* icd_parser.c
 *
 *  Created on: 2023. 11. 21.
 *      Author: hadud
 */

#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include "icd_parser.h"
#include "xparameters.h"
#include "xgpiops.h"
#include "axi_rc_spectrum.h"
#include "sleep.h"

#include "user_func.h"
#include "adrv9008.h"
#include "rf_control.h"
#include "rcrm_control.h"
#include "rcfm_control.h"
#include "Init.h"


RECV_SETTING DPU_STATUS = {0, };
RF_SETTING	 RF_STATUS = {0, };
HW_CHECK BIT_STATUS = {0, };
HW_CHECK PBIT_STATUS = {0, };
uint16_t spec_packet_size = 0;
static uint16_t	BIT_STS = 0;
static uint8_t	DPU_MODE = 1U;		//1 : CH_SWEEP		2 : CH_Fix

extern uint32_t dpu_iter_count;
extern uint32_t dpu_ref_level;
extern uint32_t dpu_win_func;


ICD_HEADER ParserTCP(uint8_t *recv_buffer, uint16_t packet_len)
{
	int Status = 0;
	uint64_t freq_buf1 = 0;
	uint64_t freq_buf2 = 0;
	ICD_HEADER	TCP_RcvBuf = {0,};
	uint8_t		tmp_adrv_gain = 0;

	TCP_RcvBuf.SRC_CODE = *recv_buffer;
	TCP_RcvBuf.DEST_CODE = *(recv_buffer + 1U);
	TCP_RcvBuf.CMD_CODE = ((uint16_t)(*(recv_buffer + 3U) << 8)) | *(recv_buffer + 2U);
	TCP_RcvBuf.DATA_SIZE = ((uint32_t)(*(recv_buffer + 7U) << 24)) | ((uint32_t)(*(recv_buffer + 6U) << 16)) | ((uint32_t)(*(recv_buffer + 5) << 8)) | *(recv_buffer + 4);


	uint32_t OPCODE = ((uint32_t)TCP_RcvBuf.SRC_CODE << 24) | ((uint32_t)TCP_RcvBuf.DEST_CODE << 16) | ((uint32_t)TCP_RcvBuf.CMD_CODE);


//DPU_CTRL Sector
	if(OPCODE == SET_CENTER_FREQ){
		freq_buf1 = ((uint64_t)(*(recv_buffer + 15U) << 24)) | ((uint64_t)(*(recv_buffer + 14U) << 16)) | ((uint64_t)(*(recv_buffer + 13U) << 8)) | ((uint64_t)(*(recv_buffer + 12U) << 0));
		freq_buf2 = ((uint64_t)(*(recv_buffer + 11U) << 24)) | ((uint64_t)(*(recv_buffer + 10U) << 16)) | ((uint64_t)(*(recv_buffer + 9U) << 8)) | ((uint64_t)(*(recv_buffer + 8U) << 0));
		DPU_STATUS.CenterFreq = (uint64_t)(((freq_buf1 & 0xFFFFFFFFU) << 32) | (freq_buf2 & 0xFFFFFFFFU));

		//RF BD CTRL
		SetRcrmStatFreq(DPU_STATUS.CenterFreq + FREQ_OFFSET);
		printf("Set Center Frequency : %luHz\r\n", DPU_STATUS.CenterFreq);

		//ADRV Set
		tal.devHalInfo = (void *) &hal;
		Status = ChangeLoFreq(&tal, DPU_STATUS.CenterFreq + FREQ_OFFSET - FREQ_NCO);
		if (Status != XST_SUCCESS) {
			(void)memset(&TCP_RcvBuf, 0x00, sizeof(ICD_HEADER));
			return TCP_RcvBuf;
		}
	}
//BW Fix(50MHz)
	else if(OPCODE == SET_BW){
		switch(*(recv_buffer + 8U)){
			case 2U:
				DPU_STATUS.BandWidth = BW_50M;
				DPU_STATUS.ParmBw = 2U;
				break;
			default :
				DPU_STATUS.BandWidth = BW_50M;
				DPU_STATUS.ParmBw = 2U;
				break;
		}
		printf("Set BW : %luHz\r\n", DPU_STATUS.BandWidth);
	}

	else if(OPCODE == SET_RBW){
		//Spectrum AP Stop
		(void)rts_end(RC_SPCTRUM_BaseAddr);

		switch(*(recv_buffer + 8U)){
			case 2U:								//RBW_30kHz
				DPU_STATUS.RBW = RBW_30kHz;
				spec_packet_size = FFT_2048_BIN + ICD_HEADER_SIZE + SPEC_HEADER_SIZE;		//Actual FFT Size = 8192, Resize to 2048(240222)
				DPU_STATUS.ParmRbw = 2U;
				DPU_STATUS.SpecBin = FFT_2048_BIN/2U;

				RTS_SPECTRUM_CTRL_mWriteReg(RC_SPCTRUM_BaseAddr, REG_RTS_FFT_SCALE, 0x1AAB);	//0x2AB	0x6AB
				RTS_SPECTRUM_CTRL_mWriteReg(RC_SPCTRUM_BaseAddr, REG_RTS_FFT_nFFT, 0xD);
				RTS_SPECTRUM_CTRL_mWriteReg(RC_SPCTRUM_BaseAddr, REG_FFT_SIZE, 0x2);
				break;
			case 4U:								//RBW_120kHz
				DPU_STATUS.RBW = RBW_120kHz;
				spec_packet_size = FFT_512_BIN + ICD_HEADER_SIZE + SPEC_HEADER_SIZE;		//Actual FFT Size = 2048, Resize to 512(240222)
				DPU_STATUS.ParmRbw = 4U;
				DPU_STATUS.SpecBin = FFT_512_BIN/2U;

				RTS_SPECTRUM_CTRL_mWriteReg(RC_SPCTRUM_BaseAddr, REG_RTS_FFT_SCALE, 0x06AB);	//0x2AB	0x6AB
				RTS_SPECTRUM_CTRL_mWriteReg(RC_SPCTRUM_BaseAddr, REG_RTS_FFT_nFFT, 0xB);
				RTS_SPECTRUM_CTRL_mWriteReg(RC_SPCTRUM_BaseAddr, REG_FFT_SIZE, 0x1);
				break;
			case 5U:								//RBW_240kHz
				DPU_STATUS.RBW = RBW_240kHz;
				spec_packet_size = FFT_256_BIN + ICD_HEADER_SIZE + SPEC_HEADER_SIZE;		//Actual FFT Size = 1024, Resize to 256(240222)
				DPU_STATUS.ParmRbw = 5U;
				DPU_STATUS.SpecBin = FFT_256_BIN/2U;

				RTS_SPECTRUM_CTRL_mWriteReg(RC_SPCTRUM_BaseAddr, REG_RTS_FFT_SCALE, 0x02AB);	//0x2AB	0x6AB
				RTS_SPECTRUM_CTRL_mWriteReg(RC_SPCTRUM_BaseAddr, REG_RTS_FFT_nFFT, 0xA);
				RTS_SPECTRUM_CTRL_mWriteReg(RC_SPCTRUM_BaseAddr, REG_FFT_SIZE, 0x0);
				break;
			default :							//RBW_30kHz
				DPU_STATUS.RBW = RBW_30kHz;
				spec_packet_size = FFT_2048_BIN + ICD_HEADER_SIZE + SPEC_HEADER_SIZE;		//Default RBW = 30kHz
				DPU_STATUS.ParmRbw = 2U;
				DPU_STATUS.SpecBin = FFT_2048_BIN/2U;
				break;
		}
		(void)rts_start(RC_SPCTRUM_BaseAddr, dpu_iter_count, dpu_ref_level, dpu_win_func);
		(void)rts_end(RC_SPCTRUM_BaseAddr);
		(void)rts_start(RC_SPCTRUM_BaseAddr, dpu_iter_count, dpu_ref_level, dpu_win_func);

		printf("Set RBW : %dHz\r\n", DPU_STATUS.RBW);
	}

	else if(OPCODE == SET_SPEC_START){
		if((*(recv_buffer + 8U)) == 1U){			//CH Sweep Mode
			tal.devHalInfo = (void *) &hal;

			Status = HoppingStart(&tal, 433000000U + FREQ_OFFSET - FREQ_NCO);
			if (Status != XST_SUCCESS) {
				printf("ADRV Freq Hopping Start Fail\r\n");
				(void)memset(&TCP_RcvBuf, 0x00, sizeof(ICD_HEADER));
				return TCP_RcvBuf;
			}

			DPU_STATUS.ScanMode = 0x01U;
		}
		else{								//CH Fix Mode(50MHz) for DPU Board TEST
			DPU_STATUS.ScanMode = 0x02U;
		}

		DPU_STATUS.START = (*(recv_buffer + 8U));
		DPU_MODE = (*(recv_buffer + 8U));
		printf("Spectrum Analysis Start.\r\n");
	}

	else if(OPCODE == SET_SPEC_STOP){
		if(DPU_MODE == 1U){
			Status = HoppingEnd(&tal, 433000000U + FREQ_OFFSET - FREQ_NCO);
			if (Status != XST_SUCCESS) {
				printf("ADRV Freq Hopping End Fail\r\n");
				(void)memset(&TCP_RcvBuf, 0x00, sizeof(ICD_HEADER));
				return TCP_RcvBuf;
			}
		}
		else{}

		DPU_STATUS.START = (*(recv_buffer + 8U));
		DPU_STATUS.ScanMode = 0x00;

		printf("Spectrum Analysis Stop.\r\n");
	}

	else if(OPCODE == SET_REQUEST_BIT){
		if((*(recv_buffer + 8U)) == 1U){
		}
		else{
			GetStatusIBIT();
		}
		BIT_STATUS.BIT_SET = 1U;
	}
	else if(OPCODE == TEST_ITER_CNT){
		DPU_STATUS.IterCnt = (*(recv_buffer + 8U));
	}
	else if(OPCODE == TEST_REF_LEVEL){
		dpu_ref_level = ((uint32_t)(*(recv_buffer + 11U)) << 24) | ((uint32_t)(*(recv_buffer + 10U)) << 16) | ((uint32_t)(*(recv_buffer + 9U)) << 8) | ((uint32_t)(*(recv_buffer + 8U)) << 0);

		(void)set_ref_level(RC_SPCTRUM_BaseAddr, dpu_ref_level);
	}
	else if(OPCODE == TEST_ADRV_GAIN){
		tmp_adrv_gain = (*(recv_buffer + 8U));
		tmp_adrv_gain = (uint8_t)(255U - tmp_adrv_gain);
		//ADRV Set
		tal.devHalInfo = (void *) &hal;
		Status = SetAdrvGain(&tal, tmp_adrv_gain);
		if (Status != XST_SUCCESS) {
			(void)memset(&TCP_RcvBuf, 0x00, sizeof(ICD_HEADER));
			return TCP_RcvBuf;
		}
	}
	//RF_CTRL Sector
	else if(OPCODE == SET_RCV_FREQ){
		if((*(recv_buffer + 8U)) == 0U){
			freq_buf1 = ((uint64_t)(*(recv_buffer + 16U)) << 24) | ((uint64_t)(*(recv_buffer + 15U)) << 16) | ((uint64_t)(*(recv_buffer + 14U)) << 8) | ((uint64_t)(*(recv_buffer + 13U)) << 0);
			freq_buf2 = ((uint64_t)(*(recv_buffer + 12U)) << 24) | ((uint64_t)(*(recv_buffer + 11U)) << 16) | ((uint64_t)(*(recv_buffer + 10U)) << 8) | ((uint64_t)(*(recv_buffer + 9U)) << 0);
			RF_STATUS.RCV_FREQ = (uint64_t)(((freq_buf1 & 0xFFFFFFFFU) << 32) | (freq_buf2 & 0xFFFFFFFFU));

			SetRcrmStatFreq(RF_STATUS.RCV_FREQ + FREQ_OFFSET);
		}
		else{}
	}
	else if(OPCODE == SET_FILTER_PATH){
		if((*(recv_buffer + 8U)) == 0U){
			if((*(recv_buffer + 9U)) == 0U){
				rcrm_status.rcrm_bpf_bank = (*(recv_buffer + 10U));
			}
			else{
				rcrm_status.rcrm_lpf_bank = (*(recv_buffer + 10U));
			}

			MakeRcrmSpiCmdModeCtrl();
		}
		else{}
	}
	else if(OPCODE == SET_AMP_MODE){
		if((*(recv_buffer + 8U)) == 0U){
			if((*(recv_buffer + 9U)) == RCFM){
				RF_STATUS.RCFM_LNA_MODE = (*(recv_buffer + 10U));

				SetRcfmStatAmpFst(RF_STATUS.RCFM_LNA_MODE);
			}
			else if((*(recv_buffer + 9U)) == RCRM){
				RF_STATUS.RCRM_LNA_MODE = (*(recv_buffer + 10U));

				SetRcrmStatAmpScd(RF_STATUS.RCRM_LNA_MODE);
			}
			else{}


			if((RF_STATUS.RCFM_LNA_MODE & RF_STATUS.RCRM_LNA_MODE) == 1U){
				dpu_ref_level = 0x8000000AU;
			}
			else if((RF_STATUS.RCFM_LNA_MODE | RF_STATUS.RCRM_LNA_MODE) == 0U){
				dpu_ref_level = 0x80000000U;
			}
			else{
				dpu_ref_level = 0x80000005U;
			}

		}
		else{}
	}

	else if(OPCODE == SET_RCV_ATTEN){
		if((*(recv_buffer + 8U)) == 0U){
			if((*(recv_buffer + 9U)) == SYS_ATTEN){
				RF_STATUS.ATTEN_SYS = (*(recv_buffer + 10U));

				SetRcrmStatSysAtt(RF_STATUS.ATTEN_SYS);
			}
			else if((*(recv_buffer + 9U)) == GAIN_ATTEN){
				RF_STATUS.ATTEN_GAIN = (*(recv_buffer + 10U));

				SetRcrmStatGainAtt(RF_STATUS.ATTEN_GAIN);
			}
			else{}
		}
		else{}
	}
	else if(OPCODE == SET_RCV_PATH){
		if((*(recv_buffer + 8U)) == 0U){
			RF_STATUS.RCV_PATH = (*(recv_buffer + 9U));

			SetRcfmStatPath(RF_STATUS.RCV_PATH);
		}
		else{}
	}
	else if(OPCODE == SET_BIT_CTRL){
		if((*(recv_buffer + 8U)) == 0U){
			RF_STATUS.RF_BIT_EN = (*(recv_buffer + 8U));
			SetRcfmStatBitEn(RF_STATUS.RF_BIT_EN);

			freq_buf1 = ((uint64_t)(*(recv_buffer + 17U)) << 24) | ((uint64_t)(*(recv_buffer + 16U)) << 16) | ((uint64_t)(*(recv_buffer + 15U)) << 8) | ((uint64_t)(*(recv_buffer + 14U)) << 0);
			freq_buf2 = ((uint64_t)(*(recv_buffer + 13U)) << 24) | ((uint64_t)(*(recv_buffer + 12U)) << 16) | ((uint64_t)(*(recv_buffer + 11U)) << 8) | ((uint64_t)(*(recv_buffer + 10U)) << 0);
			RF_STATUS.RF_BIT_FREQ = (uint64_t)(((freq_buf1 & 0xFFFFFFFFU) << 32) | (freq_buf2 & 0xFFFFFFFFU));

			SetRcfmStatBitFreq(RF_STATUS.RF_BIT_FREQ);

			if((*(recv_buffer + 9U)) == 0U){
				SPI_WriteReg(LMX2592, 0x00U, 0X221DU, 3U);
			}
			else{}
		}
		else{}
	}
	else if(OPCODE == SET_ANT_PATH){
		if((*(recv_buffer + 8U)) == 0U){
			RF_STATUS.ANT_PATH = (*(recv_buffer + 9U));

			SetRcfmStatPathANT(RF_STATUS.ANT_PATH);
		}
		else{}
	}
	else if(OPCODE == SET_LNA_MODE){
		if((*(recv_buffer + 8U)) == 0U){
			if((*(recv_buffer + 9U)) == AMP_BYPASS){
				RF_STATUS.RCFM_LNA_MODE = 0;
				RF_STATUS.RCRM_LNA_MODE = 0;

				dpu_ref_level = 0x80000000U;		//REF_LEV	0dB
				(void)set_ref_level(RC_SPCTRUM_BaseAddr, dpu_ref_level);
			}
			else if((*(recv_buffer + 9U)) == AMP_MODE_1){
				RF_STATUS.RCFM_LNA_MODE = 0U;
				RF_STATUS.RCRM_LNA_MODE = 1U;

				dpu_ref_level = 0x80000005U;		//REF_LEV	20dB
				(void)set_ref_level(RC_SPCTRUM_BaseAddr, dpu_ref_level);
			}
			else if((*(recv_buffer + 9U)) == AMP_MODE_2){
				RF_STATUS.RCFM_LNA_MODE = 1U;
				RF_STATUS.RCRM_LNA_MODE = 1U;

				dpu_ref_level = 0x8000000AU;		//REF_LEV	40dB
				(void)set_ref_level(RC_SPCTRUM_BaseAddr, dpu_ref_level);
			}
			else{}

			SetRcfmStatAmpFst(RF_STATUS.RCFM_LNA_MODE);
			SetRcrmStatAmpScd(RF_STATUS.RCRM_LNA_MODE);
		}
		else{}
	}
	else if(OPCODE == SET_JAM_START){
		if((*(recv_buffer + 8U)) == 0U){
			if((*(recv_buffer + 9U)) == JAM_START){
				RF_STATUS.JAM_RCV_PATH = RF_STATUS.RCV_PATH;
				RF_STATUS.JAM_ANT_PATH = RF_STATUS.ANT_PATH;
				RF_STATUS.JAM_RCFM_LNA_MODE = RF_STATUS.RCFM_LNA_MODE;
				RF_STATUS.JAM_RCRM_LNA_MODE = RF_STATUS.RCRM_LNA_MODE;

				RF_STATUS.RCV_PATH = 0x01U;			//BIT PATH
				RF_STATUS.ANT_PATH = 0x00U;			//LAN OFF
				RF_STATUS.RCFM_LNA_MODE = 0x00U;		//LNA BYPASS Mode
				RF_STATUS.RCRM_LNA_MODE = 0x00U;		//LNA BYPASS Mode

				SetRcfmStatPath(RF_STATUS.RCV_PATH);
				SetRcfmStatPathANT(RF_STATUS.ANT_PATH);
				SetRcfmStatAmpFst(RF_STATUS.RCFM_LNA_MODE);
				SetRcrmStatAmpScd(RF_STATUS.RCRM_LNA_MODE);

				dpu_ref_level = 0x80000000U;		//REF_LEV	0dB
				(void)set_ref_level(RC_SPCTRUM_BaseAddr, dpu_ref_level);

				printf("RC_RCV : JAM mode success.\r\n");
			}
			else{
				printf("RC_RCV : JAM mode fail.\r\n");
			}
		}
		else{}
	}
	else if(OPCODE == SET_JAM_STOP){
		if((*(recv_buffer + 8U)) == 0U){
			if((*(recv_buffer + 9U)) == JAM_STOP){
				//Restore RC_RCV setting
				RF_STATUS.RCV_PATH = RF_STATUS.JAM_RCV_PATH;
				RF_STATUS.ANT_PATH = RF_STATUS.JAM_ANT_PATH;
				RF_STATUS.RCFM_LNA_MODE = RF_STATUS.JAM_RCFM_LNA_MODE;
				RF_STATUS.RCRM_LNA_MODE = RF_STATUS.JAM_RCRM_LNA_MODE;

				SetRcfmStatPath(RF_STATUS.RCV_PATH);
				SetRcfmStatPathANT(RF_STATUS.ANT_PATH);
				SetRcfmStatAmpFst(RF_STATUS.RCFM_LNA_MODE);
				SetRcrmStatAmpScd(RF_STATUS.RCRM_LNA_MODE);

				if((RF_STATUS.RCFM_LNA_MODE == 0U) && (RF_STATUS.RCRM_LNA_MODE == 0U)){
					dpu_ref_level = 0x80000000U;		//REF_LEV	0dB
					(void)set_ref_level(RC_SPCTRUM_BaseAddr, dpu_ref_level);
				}
				else if((RF_STATUS.RCFM_LNA_MODE == 0U) && (RF_STATUS.RCRM_LNA_MODE == 1U)){
					dpu_ref_level = 0x80000005U;		//REF_LEV	20dB
					(void)set_ref_level(RC_SPCTRUM_BaseAddr, dpu_ref_level);
				}
				else{
					dpu_ref_level = 0x8000000AU;		//REF_LEV	40dB
					(void)set_ref_level(RC_SPCTRUM_BaseAddr, dpu_ref_level);
				}

				printf("RC_RCV : RCV mode success.\r\n");
			}
			else{
				printf("RC_RCV : RCV mode fail.\r\n");
			}
		}
		else{}
	}
	else { }

return TCP_RcvBuf;
}

void SwapOPCODE(uint8_t* buf){
	uint8_t tmp = 0;

	tmp = (*(buf));
	(*(buf)) = (*(buf + 1U));
	(*(buf + 1U)) = tmp;
}

