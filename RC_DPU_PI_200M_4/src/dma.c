/*
 * dma.c
 *
 *  Created on: 2023. 10. 17.
 *      Author: hadud
 */


/******************************************************************************
*
* Copyright (C) 2010 - 2019 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/*****************************************************************************/
/**
 *
 * @file xaxidma_example_sg_poll.c
 *
 * This file demonstrates how to use the xaxidma driver on the Xilinx AXI
 * DMA core (AXIDMA) to transfer packets in polling mode when the AXIDMA
 * core is configured in Scatter Gather Mode.
 *
 * This code assumes a loopback hardware widget is connected to the AXI DMA
 * core for data packet loopback.
 *
 * To see the debug print, you need a Uart16550 or uartlite in your system,
 * and please set "-DDEBUG" in your compiler options. You need to rebuild your
 * software executable.
 *
 * Make sure that MEMORY_BASE is defined properly as per the HW system. The
 * h/w system built in Area mode has a maximum DDR memory limit of 64MB. In
 * throughput mode, it is 512MB.  These limits are need to ensured for
 * proper operation of this code.
 *
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00a jz   05/17/10 First release
 * 2.00a jz   08/10/10 Second release, added in xaxidma_g.c, xaxidma_sinit.c,
 *                     updated tcl file, added xaxidma_porting_guide.h, removed
 *                     workaround for endianness
 * 4.00a rkv  02/22/11 Name of the file has been changed for naming consistency
 *       	       	   Added interrupt support for ARM.
 * 5.00a srt  03/05/12 Added Flushing and Invalidation of Caches to fix CRs
 *		       		   648103, 648701.
 *		       		   Added V7 DDR Base Address to fix CR 649405.
 * 6.00a srt  03/27/12 Changed API calls to support MCDMA driver.
 * 7.00a srt  06/18/12 API calls are reverted back for backward compatibility.
 * 7.01a srt  11/02/12 Buffer sizes (Tx and Rx) are modified to meet maximum
 *		       DDR memory limit of the h/w system built with Area mode
 * 7.02a srt  03/01/13 Updated DDR base address for IPI designs (CR 703656).
 * 9.1   adk  01/07/16 Updated DDR base address for Ultrascale (CR 799532) and
 *		       removed the defines for S6/V6.
 * 9.2   vak  15/04/16 Fixed compilation warnings in th example
 * 9.3   ms   01/23/17 Modified xil_printf statement in main function to
 *                     ensure that "Successfully ran" and "Failed" strings are
 *                     available in all examples. This is a fix for CR-965028.
 * 9.9   rsp  01/21/19 Fix use of #elif check in deriving DDR_BASE_ADDR.
 * </pre>
 *
 * ***************************************************************************
 */
/***************************** Include Files *********************************/
#include <stdio.h>
#include "xaxidma.h"
#include "xparameters.h"
#include "xdebug.h"
#include "dma.h"
#include "icd_parser.h"
#include <sleep.h>

#ifdef __aarch64__
#include "xil_mmu.h"
#endif

#if (!defined(DEBUG))
extern void xil_printf(const char *format, ...);
#endif

/******************** Constant Definitions **********************************/

/*
 * Device hardware build related constants.
 */

#define DDR_BASE_ADDR	XPAR_PSU_DDR_0_S_AXI_BASEADDR
#define MEM_BASE_ADDR		(DDR_BASE_ADDR + 0x1000000)
#define RX_BD_SPACE_HIGH	(MEM_BASE_ADDR + 0x00001FFF)
#define RX_BUFFER_HIGH		(MEM_BASE_ADDR + 0x004FFFFF)
#define MARK_UNCACHEABLE    0x701
#define TEST_START_VALUE	0xC
#define POLL_TIMEOUT_COUNTER	1000000U


int RxSetup(XAxiDma * AxiDmaInstPtr);
//static int RxDmaData(void);
static int Rcv_Dma(void);

volatile static uint8_t RX_BD_SPACE_BASE[RX_BD_SPACE_SIZE] __attribute__ ((aligned (64)));
volatile static uint8_t RX_BUFFER_BASE[RX_BUFFER_SIZE + ICD_HEADER_SIZE + SPEC_HEADER_SIZE] __attribute__ ((aligned (64)));


XAxiDma AxiDma;
volatile int RxDone;

uint8_t DataReady;
uint32_t *AddrSpecData;
uint32_t *AddrSpecHeader;
extern uint8_t SendDone;
extern RECV_SETTING DPU_STATUS;

uint32_t *AddrSpecPrevHeader;
uint32_t *AddrSpecCurHeader;

extern uint8_t SPEC_BUF_PREV[FFT_2048_BIN + ICD_HEADER_SIZE + SPEC_HEADER_SIZE];
extern uint8_t SPEC_BUF_CUR[FFT_2048_BIN + ICD_HEADER_SIZE + SPEC_HEADER_SIZE];

/*****************************************************************************/
/**
*
* Main function
*
* This function is the main entry of the tests on DMA core. It sets up
* DMA engine to be ready to receive and send packets, then a packet is
* transmitted and will be verified after it is received via the DMA loopback
* widget.
*
* @param	None
*
* @return
*		- XST_SUCCESS if test passes
*		- XST_FAILURE if test fails.
*
* @note		None.
*
******************************************************************************/
int Init_DMA(uint32_t dev_id)
{
	int Status;
	XAxiDma_Config *Config;

	Xil_SetTlbAttributes(RX_BD_SPACE_BASE, MARK_UNCACHEABLE);

	RxDone = 0;

	Config = XAxiDma_LookupConfig(dev_id);
	if (!Config) {
		return XST_FAILURE;
	}

	/* Initialize DMA engine */
	Status = XAxiDma_CfgInitialize(&AxiDma, Config);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	printf("***** DMA Reset ... ");
	XAxiDma_Reset(&AxiDma);		//�迵�� ������ �ڵ�
	printf("Success *****\n");

	Status = RxSetup(&AxiDma);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	xil_printf("--- DMA Init Complete ---\r\n");

	return XST_SUCCESS;
}


/*****************************************************************************/
/**
*
* This function sets up RX channel of the DMA engine to be ready for packet
* reception
*
* @param	AxiDmaInstPtr is the pointer to the instance of the DMA engine.
*
* @return	XST_SUCCESS if the setup is successful, XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
int RxSetup(XAxiDma * AxiDmaInstPtr)
{
	XAxiDma_BdRing *RxRingPtr;
	int Delay = 0;
	int Coalesce = 1;
	int Status;
	XAxiDma_Bd BdTemplate;
	XAxiDma_Bd *BdPtr;
	XAxiDma_Bd *BdCurPtr;
	u32 BdCount;
	u32 FreeBdCount;
	UINTPTR RxBufferPtr;
	int Index;

	SendDone = 1;
	RxRingPtr = XAxiDma_GetRxRing(&AxiDma);

	/* Disable all RX interrupts before RxBD space setup */

	XAxiDma_BdRingIntDisable(RxRingPtr, XAXIDMA_IRQ_ALL_MASK);

	/* Set delay and coalescing */
	XAxiDma_BdRingSetCoalesce(RxRingPtr, Coalesce, Delay);

	/* Setup Rx BD space */
	BdCount = XAxiDma_BdRingCntCalc(XAXIDMA_BD_MINIMUM_ALIGNMENT, RX_BD_SPACE_SIZE);

	Status = XAxiDma_BdRingCreate(RxRingPtr, (UINTPTR)RX_BD_SPACE_BASE, (UINTPTR)RX_BD_SPACE_BASE, XAXIDMA_BD_MINIMUM_ALIGNMENT, BdCount);
	if (Status != XST_SUCCESS) {
		xil_printf("RX create BD ring failed %d\r\n", Status);

		return XST_FAILURE;
	}

	/*
	 * Setup an all-zero BD as the template for the Rx channel.
	 */
	XAxiDma_BdClear(&BdTemplate);
	Status = XAxiDma_BdRingClone(RxRingPtr, &BdTemplate);
	if (Status != XST_SUCCESS) {
		xil_printf("RX clone BD failed %d\r\n", Status);

		return XST_FAILURE;
	}

	/* Attach buffers to RxBD ring so we are ready to receive packets */

	FreeBdCount = XAxiDma_BdRingGetFreeCnt(RxRingPtr);
	/* Add BD in free group to the pre-process group */
	Status = XAxiDma_BdRingAlloc(RxRingPtr, FreeBdCount, &BdPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("RX alloc BD failed %d\r\n", Status);

		return XST_FAILURE;
	}

	BdCurPtr = BdPtr;
	RxBufferPtr = (UINTPTR)&RX_BUFFER_BASE[ICD_HEADER_SIZE + SPEC_HEADER_SIZE];

	for (Index = 0; Index < FreeBdCount; Index++) {
		Status = XAxiDma_BdSetBufAddr(BdCurPtr, RxBufferPtr);
		if (Status != XST_SUCCESS) {
			xil_printf("Set buffer addr %x on BD %x failed %d\r\n",
			    (unsigned int)RxBufferPtr,
			    (UINTPTR)BdCurPtr, Status);

			return XST_FAILURE;
		}

		Status = XAxiDma_BdSetLength(BdCurPtr, MAX_PKT_LEN, RxRingPtr->MaxTransferLen);

		if (Status != XST_SUCCESS) {
			xil_printf("Rx set length %d on BD %x failed %d\r\n",
			    MAX_PKT_LEN, (UINTPTR)BdCurPtr, Status);

			return XST_FAILURE;
		}

		/* Receive BDs do not need to set anything for the control
		 * The hardware will set the SOF/EOF bits per stream status
		 */
		XAxiDma_BdSetCtrl(BdCurPtr, 0);
		XAxiDma_BdSetId(BdCurPtr, RxBufferPtr);

		RxBufferPtr += MAX_PKT_LEN;
		BdCurPtr = (XAxiDma_Bd *)XAxiDma_BdRingNext(RxRingPtr, BdCurPtr);
	}

	/* Clear the receive buffer, so we can verify data
	 */
//	memset((void *)RX_BUFFER_BASE, 0, MAX_PKT_LEN);
	memset((uint8_t *)&RX_BUFFER_BASE[ICD_HEADER_SIZE + SPEC_HEADER_SIZE], 0, MAX_PKT_LEN);


	Status = XAxiDma_BdRingToHw(RxRingPtr, FreeBdCount, BdPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("RX submit hw failed %d\r\n", Status);

		return XST_FAILURE;
	}

	/* Start RX DMA channel */
	Status = XAxiDma_BdRingStart(RxRingPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("RX start hw failed %d\r\n", Status);

		return XST_FAILURE;
	}

	return XST_SUCCESS;
}




int RxDmaData()
{
	XAxiDma_BdRing *RxRingPtr;
	XAxiDma_Bd *BdPtr;
	XAxiDma_Bd *BdCurPtr;
	int ProcessedBdCount;
	int FreeBdCount;
	int Status;
	int TimeOut = POLL_TIMEOUT_COUNTER;
	int BdCount;
	uint8_t *RxPacketPtr;
	uint32_t RxPacketSize;
	uint32_t BdSts;

	RxRingPtr = XAxiDma_GetRxRing(&AxiDma);

	while ((BdCount = XAxiDma_BdRingFromHw(RxRingPtr, XAXIDMA_ALL_BDS, &BdPtr)) == 0) {
	}

//	DataReady = 0;

	if(SendDone == 1U){
		/* Receive DMA Data */
		BdCurPtr = BdPtr;
		for(int i = 0; i < BdCount; i++){
			BdSts = XAxiDma_BdGetSts(BdCurPtr);
			if ((BdSts & XAXIDMA_BD_STS_ALL_ERR_MASK) || (!(BdSts & XAXIDMA_BD_STS_COMPLETE_MASK))) {
		//		Error = -1;
				break;
			}

			RxPacketPtr = (uint8_t *)XAxiDma_BdRead(BdCurPtr, XAXIDMA_BD_BUFA_OFFSET);
			RxPacketSize = (uint32_t)XAxiDma_BdRead(BdCurPtr, XAXIDMA_BD_STS_OFFSET) & 0x3FFFFFFU;
			Xil_DCacheInvalidateRange((UINTPTR)RxPacketPtr, RxPacketSize);

			BdCurPtr = (XAxiDma_Bd *)XAxiDma_BdRingNext(RxRingPtr, BdCurPtr);
			RxDone += 1;
		}
		if(BdCount > 0){
			DataReady = 0;
			/* Free all processed RX BDs for future transmission */
			Status = XAxiDma_BdRingFree(RxRingPtr, BdCount, BdPtr);
			if (Status != XST_SUCCESS) {
				xil_printf("Failed to free %d rx BDs %d\r\n",
					ProcessedBdCount, Status);
				return XST_FAILURE;
			}

			/* Return processed BDs to RX channel so we are ready to receive new
			 * packets:
			 *    - Allocate all free RX BDs
			 *    - Pass the BDs to RX channel
			 */
			FreeBdCount = XAxiDma_BdRingGetFreeCnt(RxRingPtr);
			Status = XAxiDma_BdRingAlloc(RxRingPtr, FreeBdCount, &BdPtr);
			if (Status != XST_SUCCESS) {
				xil_printf("bd alloc failed\r\n");
				return XST_FAILURE;
			}

//			uint32_t *BUFFER_ADDR = (uint32_t *)RX_BUFFER_BASE;
//			uint16_t BUFFER_SIZE = 3200;
//
//			transfer_data(BUFFER_ADDR, BUFFER_SIZE);
//			memset(AddrSpecData, 0x00, sizeof(SPEC_BUF_PREV) - (ICD_HEADER_SIZE + SPEC_HEADER_SIZE));

			AddrSpecData = (uint32_t *)&RX_BUFFER_BASE[ICD_HEADER_SIZE + SPEC_HEADER_SIZE];

			AddrSpecHeader = AddrSpecData - ((ICD_HEADER_SIZE + SPEC_HEADER_SIZE)/4U);
			AddrSpecPrevHeader = (uint32_t *)&SPEC_BUF_PREV;
			AddrSpecCurHeader = (uint32_t *)&SPEC_BUF_CUR;
//			memcpy(AddrSpecCurHeader, AddrSpecHeader, sizeof(AddrSpecCurHeader));
			if(DPU_STATUS.ScanMode == 0x01U){
				memcpy(AddrSpecPrevHeader, AddrSpecHeader, sizeof(SPEC_BUF_PREV));
//				memcpy(AddrSpecCurHeader, AddrSpecHeader, sizeof(SPEC_BUF_CUR));
			}
			else if(DPU_STATUS.ScanMode == 0x02U){
				memcpy(AddrSpecCurHeader, AddrSpecHeader, sizeof(SPEC_BUF_CUR));
			}
			else{}

			Status = XAxiDma_BdRingToHw(RxRingPtr, FreeBdCount, BdPtr);
			if (Status != XST_SUCCESS) {
				xil_printf("Submit %d rx BDs failed %d\r\n", FreeBdCount, Status);
				return XST_FAILURE;
			}
			DataReady = 1;
		}
	}
	else{}
	return BdCount;
}



