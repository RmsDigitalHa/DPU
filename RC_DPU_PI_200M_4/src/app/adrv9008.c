/**

 * \file adrv9009/src/app/headless.c

 *

 * \brief Contains example code for user integration with their application

 *

 * Copyright 2015-2017 Analog Devices Inc.

 * Released under the AD9378-AD9379 API license, for more information see the "LICENSE.txt" file in this zip file.

 *

 */

/****< Insert User Includes Here >***/

#include <adrv9008.h>
#include <error.h>
#include "delay.h"
#include "parameters.h"
#include "stdio.h"
#include "xil_types.h"
#include <stdbool.h>

#include "util.h"
#include "adi_hal.h"
#include "axi_adc_core.h"
#include "axi_dmac.h"
#include "gpio_extra.h"
#include "spi.h"
#include "spi_extra.h"
#include "app_talise.h"
#include "user_func.h"
#include "axi_rc_spectrum.h"

#ifndef ALTERA_PLATFORM
#include "xil_cache.h"
#endif
#include <talise.h>
#include <talise_config.h>
#include <app_config.h>
#include <app_clocking.h>
#include <app_jesd.h>
#include <app_transceiver.h>
#include <app_talise.h>
#include <ad9528.h>


// Prototype
int Init_ADRV9008(void);


/**********************************************************/
/**********************************************************/
/********** Talise Data Structure Initializations ********/
/**********************************************************/
/**********************************************************/




int Init_ADRV9008(void)
{
	adiHalErr_t err;
	int status;

	if((talInit.jesd204Settings.framerA.serializerLanesEnabled == 0U) && (talInit.jesd204Settings.framerA.K == 0U)
		&& (talInit.jesd204Settings.framerA.F == 0U) == 1U){
		goto error_0;
	}
	else{}


	// compute the lane rate from profile settings
	// lane_rate = input_rate * M * 20 / L
	// where L and M are explained in taliseJesd204bFramerConfig_t comments
	uint32_t rx_lane_rate_khz = (uint32_t)talInit.rx.rxProfile.rxOutputRate_kHz *
	    talInit.jesd204Settings.framerA.M * (20 /
	    hweight8(talInit.jesd204Settings.framerA.serializerLanesEnabled));
	uint32_t rx_div40_rate_hz = rx_lane_rate_khz * (1000U / 40U);
	uint32_t device_clk_khz = talInit.rx.rxProfile.rxOutputRate_kHz;


	printf("**********ADRV9008-1 JSED204b Test**********\n");
	printf("DEV_CLK : %d khz\n", device_clk_khz);
	printf("Lane_Rate : %d khz\n", rx_lane_rate_khz);


	// compute the local multiframe clock
	// serializer:   lmfc_rate = (lane_rate * 100) / (K * F)
	// deserializer: lmfc_rate = (lane_rate * 100) / (K * 2 * M / L)
	// where K, F, L, M are explained in taliseJesd204bFramerConfig_t comments
	uint32_t rx_lmfc_rate = (rx_lane_rate_khz * 100U) /
	(talInit.jesd204Settings.framerA.K * talInit.jesd204Settings.framerA.F);



	uint32_t lmfc_rate = rx_lmfc_rate;


	struct axi_adc_init rx_adc_init = {
		"rx_adc",
		RX_CORE_BASEADDR,
		4
	};

	struct axi_adc *rx_adc = 0U;

	//Init PS Driver
	struct xil_spi_init_param adc_spi_param = {
		.type = SPI_PS,
		.flags = SPI_CS_DECODE,
		.device_id = 0
	};

	struct xil_gpio_init_param adc_gpio_param = {
		.type = GPIO_PS,
		.device_id = GPIO_DEVICE_ID
	};


	hal.extra_gpio = &adc_gpio_param;
	hal.extra_spi = &adc_spi_param;


	tal.devHalInfo = (void *) &hal;


	/**********************************************************/
	/**********************************************************/
	/************ Talise Initialization Sequence *************/
	/**********************************************************/
	/**********************************************************/

	//AD9528 Clock Set
	err = clocking_init(rx_div40_rate_hz,
		0,
		0,
		talInit.clocks.deviceClock_kHz,
		lmfc_rate);

	if (err != ADIHAL_OK) {
		goto error_0;
	}

	//AXI_ADRV9008_RX_JESD
	err = jesd_init(rx_div40_rate_hz,
		0,
		0);

	if (err != ADIHAL_OK) {
		goto error_1;
	}


	//AXI_ADRV9008_RX_XCVR
	err = fpga_xcvr_init(rx_lane_rate_khz,
		0,
		0,
		talInit.clocks.deviceClock_kHz);

	if (err != ADIHAL_OK) {
		goto error_2;
	}

	//ADRV9008 Setup
	err = talise_setup(&tal, &talInit);
	if (err != ADIHAL_OK) {
		goto error_3;
	}

	jesd_rx_watchdog();

	/* Print JESD status */
	jesd_status();

	/* Initialize the ADC core */
	//JESD_TPL core
	status = axi_adc_init(&rx_adc, &rx_adc_init);

	status = ChangeLoFreq(&tal, 1000000000U);


	if (status == 1) {
	printf("axi_adc_init() failed with status %d\n", status);
	goto error_3;
	}

	error_3:
		fpga_xcvr_deinit();
	error_2:
		jesd_deinit();
	error_1:
		clocking_deinit();
	error_0:
		printf("Bye\n");

	return SUCCESS;
}
