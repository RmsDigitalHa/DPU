/*
 * axi_control.c
 *
 *  Created on: 2023. 10. 13.
 *      Author: hadud test
 */

#include <sys/types.h>

#include "xil_io.h"
#include "rts_spectrum_ctrl.h"
#include "axi_rc_spectrum.h"
#include "sleep.h"
//#include "dpu_regi_map.h"

//Variable
static uint32_t sts_start = 0;
static uint32_t sts_iter_count = 0;
uint16_t I_Offset = 0;
uint16_t Q_Offset = 0;


uint32_t init_rc_spectrum(){
	RTS_SPECTRUM_CTRL_mWriteReg(RC_SPCTRUM_BaseAddr, REG_RTS_START, 0);

	return 0;
}


uint32_t set_iter_count(u32 base_address, u32 iter_count)
{
	RTS_SPECTRUM_CTRL_mWriteReg(base_address, REG_RTS_ITER_COUNT, iter_count);

	return 0;
}

uint32_t set_ref_level(u32 base_address, u32 ref_level)
{
	RTS_SPECTRUM_CTRL_mWriteReg(base_address, REG_RTS_REF_LEVEL, ref_level);

	return 0;
}

uint32_t set_win_func(u32 base_address, u32 win_func)
{
	RTS_SPECTRUM_CTRL_mWriteReg(base_address, REG_RTS_WIN_FUNC, win_func);

	return 0;
}

uint32_t rts_start(u32 base_address, u32 iter_count, u32 ref_level, u32 win_func)
{
	RTS_SPECTRUM_CTRL_mWriteReg(base_address, REG_SEL_SG, 0x0);			//External Path
	RTS_SPECTRUM_CTRL_mWriteReg(base_address, REG_MULT_PARM, 0x04);		//0x04(Max) -> +12dB

	sts_iter_count = iter_count;
	set_iter_count(base_address, iter_count);
	set_ref_level(base_address, ref_level);
	set_win_func(base_address, win_func);

	RTS_SPECTRUM_CTRL_mWriteReg(base_address, REG_RTS_START, 0x1);
	usleep(100);

//	RTS_SPECTRUM_CTRL_mWriteReg(base_address, REG_RTS_DMA_START, 0x1);

//    xil_printf("+ Capturing Start\n\r");

    return 0;
}

uint32_t rts_end(u32 base_address)
{
	RTS_SPECTRUM_CTRL_mWriteReg(base_address, REG_RTS_START, 0x0);

//	RTS_SPECTRUM_CTRL_mWriteReg(base_address, REG_RTS_DMA_START, 0x0);

//    xil_printf("+ Capturing End\n\r");
	usleep(100);
    return 0;
}
