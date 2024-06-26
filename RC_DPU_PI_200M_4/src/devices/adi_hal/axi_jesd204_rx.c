/***************************************************************************//**
 *   @file   axi_jesd204_rx.c
 *   @brief  Driver for the Analog Devices AXI-JESD204-RX peripheral.
 *   @author DBogdan (dragos.bogdan@analog.com)
********************************************************************************
 * Copyright 2018(c) Analog Devices, Inc.
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

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include <error.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include "delay.h"
#include "util.h"
#include "axi_jesd204_rx.h"
#include "axi_io.h"

/******************************************************************************/
/*************************** Prototype Definitions ****************************/
/******************************************************************************/
static int32_t axi_jesd204_rx_write(struct axi_jesd204_rx *jesd, const uint32_t reg_addr, const uint32_t reg_val);
static int32_t axi_jesd204_rx_read(struct axi_jesd204_rx *jesd, const uint32_t reg_addr, uint32_t *reg_val);
int32_t axi_jesd204_rx_lane_clk_enable(struct axi_jesd204_rx *jesd);
static int32_t axi_jesd204_rx_lane_clk_disable(struct axi_jesd204_rx *jesd);
uint32_t axi_jesd204_rx_status_read(struct axi_jesd204_rx *jesd);
static int32_t axi_jesd204_rx_get_lane_errors(struct axi_jesd204_rx *jesd, const uint32_t lane, uint32_t *errors);
int32_t axi_jesd204_rx_laneinfo_read(struct axi_jesd204_rx *jesd, const uint32_t lane);
static bool axi_jesd204_rx_check_lane_status(struct axi_jesd204_rx *jesd, const uint32_t lane);
int32_t axi_jesd204_rx_watchdog(struct axi_jesd204_rx *jesd);
static int32_t axi_jesd204_rx_apply_config(struct axi_jesd204_rx *jesd, struct jesd204_rx_config *config);
int32_t axi_jesd204_rx_init(struct axi_jesd204_rx **jesd204, const struct jesd204_rx_init *init);
int32_t axi_jesd204_rx_remove(struct axi_jesd204_rx *jesd);

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/
#define JESD204_RX_REG_VERSION			0x00U
#define JESD204_RX_REG_MAGIC			0x0cU

#define JESD204_RX_REG_SYNTH_NUM_LANES			0x10U
#define JESD204_RX_REG_SYNTH_DATA_PATH_WIDTH	0x14U

#define JESD204_RX_SYNTH_REG_1 0x18U

#define JESD204_RX_REG_LINK_DISABLE		0xc0U
#define JESD204_RX_REG_LINK_STATE		0xc4U
#define JESD204_RX_REG_LINK_CLK_RATIO	0xc8U

#define JESD204_RX_REG_SYSREF_CONF		0x100U
#define JESD204_RX_REG_SYSREF_CONF_SYSREF_DISABLE	BIT(0)

#define JESD204_RX_REG_SYSREF_STATUS	0x108U

#define JESD204_RX_REG_LINK_CONF0		0x210U
#define JESD204_RX_REG_LINK_CONF1		0x214U

#define JESD204_RX_REG_LINK_CONF2		0x240U
//add 2020.10.14 LJH
#define JESD204_RX_REG_LINK_CONF3		0x244U

#define JESD204_RX_LINK_CONF2_BUFFER_EARLY_RELEASE	BIT(16)

#define JESD204_RX_REG_LINK_STATUS		0x280U

#define JESD204_RX_REG_LANE_STATUS(x)	(((x) * 32U) + 0x300U)
#define JESD204_RX_REG_LANE_LATENCY(x)	(((x) * 32U) + 0x304U)
#define JESD204_RX_REG_LANE_ERRORS(x)	(((x) * 32U) + 0x308U)
#define JESD204_RX_REG_ILAS(x, y)		((((x) * 32U) + ((y) * 4U)) + 0x310U)

#define JESD204_TX_REG_ILAS(x, y)		\
	(((x) * 32U + (y) * 4U) + 0x310U)

#define JESD204_RX_MAGIC				\
	(((uint32_t)('2') << 24) | ((uint32_t)('0') << 16) | ((uint32_t)('4') << 8) | ((uint32_t)('R')))

#define PCORE_VERSION_MAJOR(x)		((x) >> 16)
#define PCORE_VERSION_MINOR(x)		(((x) >> 8) & (uint32_t)0xff)
#define PCORE_VERSION_PATCH(x)		((x) & (uint32_t)0xff)

static const char *axi_jesd204_rx_link_status_label[] = {
	"RESET",
	"WAIT FOR PHY",
	"CGS",
	"DATA",
};

static const char *axi_jesd204_rx_lane_status_label[] = {
	"INIT",
	"CHECK",
	"DATA",
	"UNKNOWN",
};

/******************************************************************************/
/************************** Functions Implementation **************************/
/******************************************************************************/

/**
 * @brief axi_jesd204_rx_write
 */
static int32_t axi_jesd204_rx_write(struct axi_jesd204_rx *jesd,
		const uint32_t reg_addr, const uint32_t reg_val)
{
	axi_io_write(jesd->base, reg_addr, reg_val);

	return SUCCESS;
}

/**
 * @brief axi_jesd204_rx_read
 */
static int32_t axi_jesd204_rx_read(struct axi_jesd204_rx *jesd,
		const uint32_t reg_addr, uint32_t *reg_val)
{
	axi_io_read(jesd->base, reg_addr, reg_val);

	return SUCCESS;
}

/**
 * @brief axi_jesd204_rx_lane_clk_enable
 */
int32_t axi_jesd204_rx_lane_clk_enable(struct axi_jesd204_rx *jesd)
{
	(void)axi_jesd204_rx_write(jesd, JESD204_RX_REG_SYSREF_STATUS, 0x3U);
	(void)axi_jesd204_rx_write(jesd, JESD204_RX_REG_LINK_DISABLE, 0x0U);

	return SUCCESS;
}

/**
 * @brief axi_jesd204_rx_lane_clk_disable
 */
static int32_t axi_jesd204_rx_lane_clk_disable(struct axi_jesd204_rx *jesd)
{
	return axi_jesd204_rx_write(jesd, JESD204_RX_REG_LINK_DISABLE, 0x1U);
}

/**
 * @brief axi_jesd204_rx_status_read
 */
uint32_t axi_jesd204_rx_status_read(struct axi_jesd204_rx *jesd)
{
	uint32_t link_disabled;
	uint32_t link_status;
	uint32_t sysref_status;
	uint32_t clock_ratio;
	uint32_t clock_rate;
	uint32_t link_rate;
	uint32_t sysref_config;

	(void)axi_jesd204_rx_read(jesd, JESD204_RX_REG_LINK_STATE, &link_disabled);
	(void)axi_jesd204_rx_read(jesd, JESD204_RX_REG_LINK_STATUS, &link_status);
	(void)axi_jesd204_rx_read(jesd, JESD204_RX_REG_SYSREF_STATUS, &sysref_status);
	(void)axi_jesd204_rx_read(jesd, JESD204_RX_REG_LINK_CLK_RATIO, &clock_ratio);
	(void)axi_jesd204_rx_read(jesd, JESD204_RX_REG_SYSREF_CONF, &sysref_config);

	printf("%s status:\n", jesd->name);

	if((link_disabled & 0x1U) == 1U){
		printf("\tLink is disabled\n");
	} else{
		printf("\tLink is enabled\n");
	}

	if (clock_ratio == 0U) {
		printf("\tMeasured Link Clock: off\n");
	} else {
		clock_rate = DIV_ROUND_CLOSEST_ULL(100000ULL * clock_ratio,
						   1ULL << 16);
		printf("\tMeasured Link Clock: %"PRIu32".%.3"PRIu32" MHz\n",\
		       clock_rate / 1000U, clock_rate % 1000U);
	}

	clock_rate = jesd->device_clk_khz;
	printf("\tReported Link Clock: %"PRIu32".%.3"PRIu32" MHz\n",
	       clock_rate / 1000U, clock_rate % 1000U);

	if ((link_disabled & 0x1U) != 1U) {
		clock_rate = jesd->lane_clk_khz;
		link_rate = DIV_ROUND_CLOSEST(clock_rate, 40);
		printf("\tLane rate: %"PRIu32".%.3"PRIu32" MHz\n"
		       "\tLane rate / 40: %"PRIu32".%.3"PRIu32" MHz\n",
		       clock_rate / 1000U, clock_rate % 1000U,
		       link_rate / 1000U, link_rate % 1000U);

		printf("\tLink status: %s\n"
		       "\tSYSREF captured: %d (0 : Enable / 1 : Disable), %d (0 : No / 1 : Yes)\n"
		       "\tSYSREF alignment error: %d (0 : Enable / 1 : Disable), %d (0 : No / 1 : Yes)\n",
		       axi_jesd204_rx_link_status_label[link_status & 0x3U],
		       (sysref_config & JESD204_RX_REG_SYSREF_CONF_SYSREF_DISABLE),
		       (sysref_status & 1U),
		       (sysref_config & JESD204_RX_REG_SYSREF_CONF_SYSREF_DISABLE),
		       (sysref_status & 2U));
	} else {
		printf("\tExternal reset is %d (0 : Deasserted / 1 : Asserted)\n",
		       (link_disabled & 0x2U));
	}


	return SUCCESS;
}

/**
 * @brief axi_jesd204_rx_get_lane_errors
 */
static int32_t axi_jesd204_rx_get_lane_errors(struct axi_jesd204_rx *jesd,
		const uint32_t lane, uint32_t *errors)
{
	return axi_jesd204_rx_read(jesd, JESD204_RX_REG_LANE_ERRORS(lane), errors);
}

/**
 * @brief axi_jesd204_rx_laneinfo_read
 */
int32_t axi_jesd204_rx_laneinfo_read(struct axi_jesd204_rx *jesd, const uint32_t lane)
{
	uint32_t lane_status;
	uint32_t errors;
	uint32_t octets_per_multiframe;
	uint32_t lane_latency;
	uint32_t val[4];

	(void)axi_jesd204_rx_read(jesd, JESD204_RX_REG_LANE_STATUS(lane), &lane_status);

	printf("%s lane %"PRIu32" status:\n", jesd->name, lane);

	if (PCORE_VERSION_MINOR(jesd->version) >= 2U) {
		(void)axi_jesd204_rx_get_lane_errors(jesd, lane, &errors);
		printf("Errors: %"PRIu32"\n", errors);
	}

	printf("\tCGS state: %s\n",
	       axi_jesd204_rx_lane_status_label[lane_status & 0x3U]);

	printf("\tInitial Frame Synchronization: %d (0 : No / 1 : Yes)\n",
	       (lane_status & BIT(4)));
	if (((lane_status >> 4) & 0x1U) != 1U) {
		return FAILURE;
	}
	(void)axi_jesd204_rx_read(jesd, JESD204_RX_REG_LINK_CONF0, &octets_per_multiframe);
	octets_per_multiframe &= 0xffffU;
	octets_per_multiframe += 1U;

	(void)axi_jesd204_rx_read(jesd, JESD204_RX_REG_LANE_LATENCY(lane), &lane_latency);
	printf("\tLane Latency: %"PRIu32" Multi-frames and %"PRIu32" Octets\n",
	       lane_latency / octets_per_multiframe,
	       lane_latency % octets_per_multiframe);

	printf("\tInitial Lane Alignment Sequence: %d (0 : No / 1 : Yes)\n",
	       (lane_status & BIT(5)));

	if (((lane_status >> 5) & 0x1U) != 1U) {
		return FAILURE;
	}

	(void)axi_jesd204_rx_read(jesd, JESD204_RX_REG_ILAS(lane, 0U), &val[0]);
	(void)axi_jesd204_rx_read(jesd, JESD204_RX_REG_ILAS(lane, 1U), &val[1]);
	(void)axi_jesd204_rx_read(jesd, JESD204_RX_REG_ILAS(lane, 2U), &val[2]);
	(void)axi_jesd204_rx_read(jesd, JESD204_RX_REG_ILAS(lane, 3U), &val[3]);

	printf("\tDID: %"PRIu32", BID: %"PRIu32", LID: %"PRIu32", "
	       "L: %"PRIu32", SCR: %"PRIu32", F: %"PRIu32"\n",
	       (val[0] >> 16) & 0xffU,
	       (val[0] >> 24) & 0xfU,
	       (val[1] >> 0) & 0x1fU,
	       ((val[1] >> 8) & 0x1fU) + 1U,
	       (val[1] >> 15) & 0x1U,
	       ((val[1] >> 16) & 0xffU) + 1U
	      );

	printf("\tK: %"PRIu32", M: %"PRIu32", N: %"PRIu32", CS: %"PRIu32", "
	       "N': %"PRIu32", S: %"PRIu32", HD: %"PRIu32"\n",
	       ((val[1] >> 24) & 0x1fU) + 1U,
	       ((val[2] >> 0) & 0xffU) + 1U,
	       ((val[2] >> 8) & 0x1fU) + 1U,
	       (val[2] >> 14) & 0x3U,
	       ((val[2] >> 16) & 0x1fU) + 1U,
	       ((val[2] >> 24) & 0x1fU) + 1U,
	       (val[3] >> 7) & 0x1U
	      );

	printf("\tFCHK: 0x%"PRIX32", CF: %"PRIu32"\n",
	       (val[3] >> 24) & 0xffU,
	       (val[3] >> 0) & 0x1fU
	      );

	printf("\tADJCNT: %"PRIu32", PHADJ: %"PRIu32", ADJDIR: %"PRIu32", "
	       "JESDV: %"PRIu32", SUBCLASS: %"PRIu32"\n",
	       (val[0] >> 28) & 0xffU,
	       (val[1] >> 5) & 0x1U,
	       (val[1] >> 6) & 0x1U,
	       (val[2] >> 29) & 0x7U,
	       (val[2] >> 21) & 0x7U
	      );

	printf("\tFC: %"PRIu32" kHz\n", jesd->lane_clk_khz);

	return SUCCESS;
}

/**
 * @brief axi_jesd204_rx_check_lane_status
 */
static bool axi_jesd204_rx_check_lane_status(struct axi_jesd204_rx *jesd,
		const uint32_t lane)
{
	uint32_t status;
	uint32_t errors;
	char error_str[sizeof(" (4294967295 errors)")];

	(void)axi_jesd204_rx_read(jesd, JESD204_RX_REG_LANE_STATUS(lane), &status);
	status &= 0x3U;
	if (status != 0x0U) {
		return false;
	}

	(void)axi_jesd204_rx_read(jesd, JESD204_RX_REG_LANE_ERRORS(lane), &errors);
	snprintf(error_str, sizeof(error_str), " (%"PRIu32" errors)", errors);

	printf("%s: Lane %"PRIu32" desynced%s, restarting link\n",
	       jesd->name, lane, error_str);

	return true;
}

/**
 * @brief axi_jesd204_rx_watchdog
 */
int32_t axi_jesd204_rx_watchdog(struct axi_jesd204_rx *jesd)
{
	uint32_t link_disabled;
	uint32_t link_status;
	bool restart = false;
	uint32_t i;

	(void)axi_jesd204_rx_read(jesd, JESD204_RX_REG_LINK_STATE, &link_disabled);
	if (link_disabled == 1U) {
		return SUCCESS;
	}

	(void)axi_jesd204_rx_read(jesd, JESD204_RX_REG_LINK_STATUS, &link_status);
	if (link_status == 3U) {
		for (i = 0; i < jesd->num_lanes; i++) {
			restart = axi_jesd204_rx_check_lane_status(jesd, i);
		}

		if (restart) {
			(void)axi_jesd204_rx_write(jesd, JESD204_RX_REG_LINK_DISABLE, 0x1U);
			mdelay(100);
			(void)axi_jesd204_rx_write(jesd, JESD204_RX_REG_LINK_DISABLE, 0x0U);
		}
	}

	return SUCCESS;
}

/**
 * @brief axi_jesd204_rx_apply_config
 */
static int32_t axi_jesd204_rx_apply_config(struct axi_jesd204_rx *jesd,
				    struct jesd204_rx_config *config)
{
	uint32_t octets_per_multiframe;
	uint32_t multiframe_align;
	uint32_t val;

	octets_per_multiframe = (uint32_t)(config->frames_per_multiframe *
				config->octets_per_frame);

	multiframe_align = (uint32_t)1 << jesd->data_path_width;

	if ((octets_per_multiframe % multiframe_align) != 0U) {
		printf("%s: octets_per_frame * frames_per_multiframe must be a "
		       "multiple of %"PRIu32"\n", jesd->name, multiframe_align);
		return FAILURE;
	}

	val = (octets_per_multiframe - 1U);
	val |= (uint32_t)((config->octets_per_frame - 1U) << 16U);

	(void)axi_jesd204_rx_write(jesd, JESD204_RX_REG_LINK_CONF0, val);

	//JESD RX SCRAMBLER , Character Replacement DISABLE.
	(void)axi_jesd204_rx_write(jesd, JESD204_RX_REG_LINK_CONF1, 0x03U);

	//add 2020.10.14 lane error count (32bit)
	//0x7E << 8 : count only the disparity error
	(void)axi_jesd204_rx_write(jesd, JESD204_RX_REG_LINK_CONF3, ((uint16_t)0x7FU << 8U));
	(void)axi_jesd204_rx_write(jesd, JESD204_RX_REG_LINK_CONF3, (0x01U));

	if (config->subclass_version == 0U) {
		(void)axi_jesd204_rx_write(jesd, (uint32_t)JESD204_RX_REG_SYSREF_CONF,
				(uint32_t)JESD204_RX_REG_SYSREF_CONF_SYSREF_DISABLE);
		(void)axi_jesd204_rx_write(jesd, (uint32_t)JESD204_RX_REG_LINK_CONF2,
				(uint32_t)JESD204_RX_LINK_CONF2_BUFFER_EARLY_RELEASE);
	}
	return SUCCESS;
}

/**
 * @brief axi_jesd204_rx_init
 */
int32_t axi_jesd204_rx_init(struct axi_jesd204_rx **jesd204,
			    const struct jesd204_rx_init *init)
{
	struct axi_jesd204_rx *jesd;
	uint32_t magic;
	int32_t status;

	jesd = (struct axi_jesd204_rx *)malloc(sizeof(*jesd));

	jesd->name = init->name;
	jesd->base = init->base;
	jesd->device_clk_khz = init->device_clk_khz;
	jesd->lane_clk_khz = init->lane_clk_khz;

	(void)axi_jesd204_rx_read(jesd, JESD204_RX_REG_MAGIC, &magic);
	if (magic != JESD204_RX_MAGIC) {
		printf("%s: Unexpected peripheral identifier %.08"PRIX32"\n",
		       jesd->name, magic);
		goto err;
	}

	(void)axi_jesd204_rx_read(jesd, JESD204_RX_REG_VERSION, &jesd->version);
	if (PCORE_VERSION_MAJOR(jesd->version) != 1U) {
		printf("%s: Unsupported peripheral version %"
		       ""PRIu32".%"PRIu32".%"PRIu32"\n",
		       jesd->name,
		       PCORE_VERSION_MAJOR(jesd->version),
		       PCORE_VERSION_MINOR(jesd->version),
		       PCORE_VERSION_PATCH(jesd->version));
		goto err;
	}

	(void)axi_jesd204_rx_read(jesd, JESD204_RX_REG_SYNTH_NUM_LANES,
			    &jesd->num_lanes);
	(void)axi_jesd204_rx_read(jesd, JESD204_RX_REG_SYNTH_DATA_PATH_WIDTH,
			    &jesd->data_path_width);

	jesd->config.octets_per_frame = init->octets_per_frame;
	jesd->config.frames_per_multiframe = init->frames_per_multiframe;
	jesd->config.subclass_version = init->subclass;

	uint32_t temp = 0x00;
	(void)axi_jesd204_rx_read(jesd, JESD204_RX_SYNTH_REG_1, &temp);
	temp = temp >> 8;
	if(temp == 0x01U) {
		printf("Rx link : 8b/10b\n");
	}
	else {
		printf("Rx link : ???\n");
	}

	(void)axi_jesd204_rx_lane_clk_disable(jesd);

	status = axi_jesd204_rx_apply_config(jesd, &jesd->config);
	if (status != 0) {
		goto err;
	}

	*jesd204 = jesd;

	return SUCCESS;

err:
	free(jesd);

	return FAILURE;
}

/**
 * @brief axi_jesd204_rx_remove
 */
int32_t axi_jesd204_rx_remove(struct axi_jesd204_rx *jesd)
{
	free(jesd);

	return SUCCESS;
}
