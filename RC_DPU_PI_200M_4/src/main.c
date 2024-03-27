/*
 * Copyright (C) 2009 - 2019 Xilinx, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 */

#include <stdio.h>

#include "xparameters.h"
#include "xstatus.h"

#include "netif/xadapter.h"

#include "platform.h"
#include "platform_config.h"
#if defined (__arm__) || defined(__aarch64__)
#include "xil_printf.h"
#endif

#include "dma.h"
#include "adrv9008.h"
#include "axi_rc_spectrum.h"
#include "icd_parser.h"
#include "user_func.h"
#include "rf_control.h"
#include "Init.h"
#include "rcfm_control.h"

#include "lwip/tcp.h"
#include "xil_cache.h"
#include "sleep.h"

#if LWIP_IPV6==1
#include "lwip/ip.h"
#else
#if LWIP_DHCP==1
#include "lwip/dhcp.h"
#endif
#endif

#include "udp_server.h"

/* defined by each RAW mode application */
void tcp_fasttmr(void);
void tcp_slowtmr(void);


/* user definition */
#define DMA_DEV_ID		XPAR_AXIDMA_0_DEVICE_ID

/* missing declaration in lwIP */
void lwip_init(void);

#if LWIP_IPV6==0
#if LWIP_DHCP==1
extern volatile int dhcp_timoutcntr;
err_t dhcp_start(struct netif *netif);
#endif
#endif

extern volatile int TcpFastTmrFlag;
extern volatile int TcpSlowTmrFlag;

extern RECV_SETTING DPU_STATUS;
extern uint16_t spec_packet_size;

extern uint32_t dpu_iter_count;
extern uint32_t dpu_ref_level;
extern uint32_t dpu_win_func;
uint8_t SPEC_BUF_PREV[FFT_2048_BIN + ICD_HEADER_SIZE + SPEC_HEADER_SIZE] = {0, };
uint8_t SPEC_BUF_CUR[FFT_2048_BIN + ICD_HEADER_SIZE + SPEC_HEADER_SIZE] = {0, };






int main()
{
	int Status = 0;

	printf("##################################################\n");
	printf("FPGA FW Version : V0.3\n");
	printf("FPGA FW Date : 2024.03.14\n");
	printf("Detail : RF Level Mapping OK, Ready to EMI Test.\n");
	printf("##################################################\n");

	Status = Init_FPGA_CTRL();
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = Init_RF_CTRL();
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = Init_DMA(DMA_DEV_ID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = Init_ADRV9008();
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	//UDP Init
	UdpInitUser();

	DPU_STATUS.CenterFreq = 1000000000U;
	DPU_STATUS.BandWidth = 50000000U;			//Default BW = 50MHz(Fix)
	DPU_STATUS.RBW = RBW_30kHz;
	DPU_STATUS.ParmBw = 2U;						//BW_50MHz
	DPU_STATUS.ParmRbw = 2U;						//RBW_30kHz
	DPU_STATUS.SpecBin = FFT_2048_BIN/2U;		//FFT_256_BIN	FFT_1024_BIN	FFT_2048_BIN	FFT_4098_BIN	FFT_4098_BIN
	DPU_STATUS.ChNum = 0xBU;
	DPU_STATUS.IterCnt = 3U;
	spec_packet_size = FFT_2048_BIN + ICD_HEADER_SIZE + SPEC_HEADER_SIZE;		//FFT 2048
	memset((uint8_t *)&SPEC_BUF_PREV, 0x00, sizeof(SPEC_BUF_PREV));
	memset((uint8_t *)&SPEC_BUF_CUR, 0x00, sizeof(SPEC_BUF_CUR));


	/* receive and process packets */
	while (1) {
		if (TcpFastTmrFlag) {
			tcp_fasttmr();
			TcpFastTmrFlag = 0;
		}
		if (TcpSlowTmrFlag) {
			tcp_slowtmr();
			TcpSlowTmrFlag = 0;
		}
		xemacif_input(&server_netif);

		if(DPU_STATUS.START == 1U){
			Status = CHScanStart(DPU_STATUS.ChNum, DPU_STATUS.IterCnt);
			if (Status != XST_SUCCESS) {
				printf("CH Scan Fail.\r\n");

				return XST_FAILURE;
			}
		}
		else if(DPU_STATUS.START == 2U){
			Status = BWScanStart(DPU_STATUS.CenterFreq, DPU_STATUS.BandWidth, DPU_STATUS.RBW);
			if (Status != XST_SUCCESS) {
				printf("BW_50MHz Scan Fail.\r\n");

				return XST_FAILURE;
			}
		}
		else{
		}
	}

	printf("System Error. DPU Shut Down.\n");

	return 0;
}
