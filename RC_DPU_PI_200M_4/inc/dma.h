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
#define RX_BUFFER_SIZE				3360U
#define MAX_PKT_LEN					3360U
#define MARK_UNCACHEABLE    		0x701U


int Init_DMA(const uint32_t dev_id);
int RxDmaData(void);


#endif /* INC_DMA_H_ */
