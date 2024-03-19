/*
 * axi_rc_spectrum.h
 *
 *  Created on: 2023. 10. 13.
 *      Author: hadud
 */

#ifndef SRC_APP_AXI_RC_SPECTRUM_H_
#define SRC_APP_AXI_RC_SPECTRUM_H_

#include "xparameters.h"
#include "rts_spectrum_ctrl.h"
//#include "dpu_regi_map.h"

#define REG_RTS_START			RTS_SPECTRUM_CTRL_S00_AXI_SLV_REG0_OFFSET
#define REG_RTS_ITER_COUNT		RTS_SPECTRUM_CTRL_S00_AXI_SLV_REG1_OFFSET
#define REG_RTS_REF_LEVEL		RTS_SPECTRUM_CTRL_S00_AXI_SLV_REG2_OFFSET
#define REG_RTS_WIN_FUNC		RTS_SPECTRUM_CTRL_S00_AXI_SLV_REG3_OFFSET
#define REG_FFT_SIZE			RTS_SPECTRUM_CTRL_S00_AXI_SLV_REG4_OFFSET
#define REG_RTS_FRAME_DONE		RTS_SPECTRUM_CTRL_S00_AXI_SLV_REG5_OFFSET
#define REG_RTS_FFT_SCALE		RTS_SPECTRUM_CTRL_S00_AXI_SLV_REG6_OFFSET
#define REG_RTS_FFT_nFFT		RTS_SPECTRUM_CTRL_S00_AXI_SLV_REG7_OFFSET
#define REG_RTS_DMA_START		RTS_SPECTRUM_CTRL_S00_AXI_SLV_REG8_OFFSET
#define REG_SEL_SG				RTS_SPECTRUM_CTRL_S00_AXI_SLV_REG9_OFFSET
#define REG_MULT_PARM			RTS_SPECTRUM_CTRL_S00_AXI_SLV_REG10_OFFSET
//
#define REG_NCO_PINC_DATA		DPU_REGI_MAP_S00_AXI_SLV_REG0_OFFSET
#define REG_NCO_PINC_VALID		DPU_REGI_MAP_S00_AXI_SLV_REG1_OFFSET
#define REG_I_DATA_OFFSET		DPU_REGI_MAP_S00_AXI_SLV_REG2_OFFSET
#define REG_Q_DATA_OFFSET		DPU_REGI_MAP_S00_AXI_SLV_REG3_OFFSET


#define RC_SPCTRUM_BaseAddr		XPAR_SPEC_SUB_RTS_SPECTRUM_CTRL_0_S00_AXI_BASEADDR
#define DPU_REGI_BaseAddr		XPAR_DPU_REGI_MAP_0_S00_AXI_BASEADDR


extern uint16_t I_Offset;
extern uint16_t Q_Offset;

uint32_t init_rc_spectrum();
uint32_t set_iter_count(u32 base_address, u32 iter_count);
uint32_t set_ref_level(u32 base_address, u32 ref_level);
uint32_t set_win_func(u32 base_address, u32 win_func);
uint32_t rts_start(u32 base_address, u32 iter_count, u32 ref_level, u32 win_func);
uint32_t rts_end(u32 base_address);



/* Hardware description */
#define HW_ADC_SAMPLING_RATE	122880000
//#define HW_FFT_NPOINT			256
//#define HW_FFT_NPOINT			1024
#define HW_FFT_NPOINT			2048
//#define HW_FFT_NPOINT			4096
//#define HW_FFT_NPOINT			8192

//#define HW_RTS_WIDTH			210
//#define HW_RTS_WIDTH			834
#define HW_RTS_WIDTH			1668
//#define HW_RTS_WIDTH			3334
//#define HW_RTS_WIDTH			6668
#define HW_RTS_HEIGHT			1
#define HW_RTS_DEPTH			2
#define HW_RTS_GUARD_FACTOR		1.2288
#define HW_RTS_RANGE			100


#endif /* SRC_APP_AXI_RC_SPECTRUM_H_ */
