/***************************************************************************//**
 *   @file   app_jesd.c
 *   @brief  JESD setup and initialization routines.
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
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE
*******************************************************************************/
// stdlib
#include <adi_hal.h>
#include <app_jesd.h>
#include <axi_jesd204_rx.h>
#include <error.h>
#include <parameters.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <util.h>

// Prototype
adiHalErr_t jesd_init(uint32_t rx_div40_rate_hz, uint32_t tx_div40_rate_hz, uint32_t rx_os_div40_rate_hz);
void jesd_deinit(void);
void jesd_status(void);
void jesd_rx_watchdog(void);


struct axi_jesd204_rx *rx_jesd = NULL;
struct axi_jesd204_tx *tx_jesd = NULL;
struct axi_jesd204_rx *rx_os_jesd = NULL;


adiHalErr_t jesd_init(uint32_t rx_div40_rate_hz,
      uint32_t tx_div40_rate_hz,
      uint32_t rx_os_div40_rate_hz)
{
	int32_t status;
	uint32_t rx_lane_rate_khz = rx_div40_rate_hz / 1000U * 40U;
	struct jesd204_rx_init rx_jesd_init = {
		"rx_jesd",
		RX_JESD_BASEADDR,
		4,
		32,
		0,
		rx_div40_rate_hz / 1000U,
		rx_lane_rate_khz
	};

	uint32_t tx_lane_rate_khz = tx_div40_rate_hz / 1000U * 40U;


	/* Initialize JESD */
	status = axi_jesd204_rx_init(&rx_jesd, &rx_jesd_init);
	if (status != SUCCESS) {
		printf("error: %s: axi_jesd204_rx_init() failed\n", rx_jesd_init.name);
		goto error_5;
	}

	return ADIHAL_OK;

	error_5:
		axi_jesd204_rx_remove(rx_jesd);

	return ADIHAL_ERR;
}

void jesd_deinit(void)
{
	axi_jesd204_rx_remove(rx_jesd);
}

void jesd_status(void)
{
	axi_jesd204_rx_status_read(rx_jesd);
	axi_jesd204_rx_laneinfo_read(rx_jesd, 0);
	axi_jesd204_rx_laneinfo_read(rx_jesd, 1);

}

void jesd_rx_watchdog(void)
{
	axi_jesd204_rx_watchdog(rx_jesd);
}
