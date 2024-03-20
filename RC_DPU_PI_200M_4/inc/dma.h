/*
 * dma.h
 *
 *  Created on: 2023. 10. 17.
 *      Author: hadud
 */

#ifndef INC_DMA_H_
#define INC_DMA_H_

#include "axi_rc_spectrum.h"

/* user definition */
#define DMA_DEV_ID		XPAR_AXIDMA_0_DEVICE_ID


#define RX_BD_NUM					1U
#define RX_BD_SPACE_SIZE			(64U*RX_BD_NUM) 		/*   64 Bytes X 16 */
//#define RX_BUFFER_SIZE				(HW_RTS_WIDTH*HW_RTS_HEIGHT*HW_RTS_DEPTH*RX_BD_NUM)		/* 3200 x 16 Bytes */
#define RX_BUFFER_SIZE				3360U
//#define RX_BUFFER_SIZE				(HW_RTS_WIDTH*HW_RTS_HEIGHT*HW_RTS_DEPTH)				/* 3200 Bytes */
//#define MAX_PKT_LEN					(HW_RTS_WIDTH*HW_RTS_HEIGHT*HW_RTS_DEPTH + (ICD_HEADER_SIZE + SPEC_HEADER_SIZE))				/* 3200 Bytes */
//#define MAX_PKT_LEN					(HW_RTS_WIDTH*HW_RTS_HEIGHT*HW_RTS_DEPTH)				/* 3200 Bytes */
#define MAX_PKT_LEN					3360U
#define MARK_UNCACHEABLE    		0x701U


int Init_DMA(uint32_t dev_id);
int RxDmaData();


#endif /* INC_DMA_H_ */
