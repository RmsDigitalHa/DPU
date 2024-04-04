/***************************************************************************//**
 *   @file   app_transceiver.c
 *   @brief  FPGA XCVR setup and initialization routines.
 *   @author Darius Berghe (darius.berghe@analog.com)
********************************************************************************
 * Copyright 2019(c) Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  - Neither the name of Analog Devices, Inc. nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *  - The use of this software may or may not infringe the patent rights
 *    of one or more patent holders.  This license does not release you
 *    from the requirement that you obtain separate licenses from these
 *    patent holders to use this software.
 *  - Use of the software either in source or binary form, must be run
 *    on or directly connected to an Analog Devices Inc. component.
 *
 * THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT,
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, INTELLECTUAL PROPERTY RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/
// stdlibs
#include <error.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <util.h>

// xcvr
#ifdef ALTERA_PLATFORM
#include "altera_adxcvr.h"
#else
#include <axi_adxcvr.h>
#endif

// hal
#include <parameters.h>
#include <adi_hal.h>

// header
#include <app_jesd.h>

//Prototype
adiHalErr_t fpga_xcvr_init(const uint32_t rx_lane_rate_khz, uint32_t tx_lane_rate_khz,
						uint32_t rx_os_lane_rate_khz, const uint32_t device_clock);
void fpga_xcvr_deinit(void);

static struct adxcvr *rx_adxcvr;

adiHalErr_t fpga_xcvr_init(const uint32_t rx_lane_rate_khz,
   uint32_t tx_lane_rate_khz,
   uint32_t rx_os_lane_rate_khz,
   const uint32_t device_clock)
{
	int32_t status;

	struct adxcvr_init rx_adxcvr_init = {
		"rx_adxcvr",
		RX_XCVR_BASEADDR,
		0,
		3,
		1,
		1,
		rx_lane_rate_khz,
		device_clock,
	};

	/* Initialize ADXCR */
	status = adxcvr_init(&rx_adxcvr, &rx_adxcvr_init);

	if (status != SUCCESS) {
		printf("error: %s: adxcvr_init() failed\n", rx_adxcvr_init.name);
		goto error_8;
	}

	if(rx_adxcvr != NULL){
		status = adxcvr_clk_enable(rx_adxcvr);
		if (status != SUCCESS) {
			printf("error: %s: adxcvr_clk_enable() failed\n", rx_adxcvr->name);
			goto error_8;
		}
	}
	else{}
	return ADIHAL_OK;

	error_8:
		adxcvr_remove(rx_adxcvr);
	return ADIHAL_ERR;
}

void fpga_xcvr_deinit(void)
{
	if(rx_adxcvr != NULL){
		adxcvr_remove(rx_adxcvr);
	}
	else{}
}
