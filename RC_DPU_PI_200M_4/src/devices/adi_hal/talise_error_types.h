/**
 * \file talise_error_types.h
 * \brief Contains Talise data types for API Error messaging
 *
 * Talise API version: 3.6.0.5
 *
 * Copyright 2015-2017 Analog Devices Inc.
 * Released under the AD9378-AD9379 API license, for more information see the "LICENSE.txt" file in this zip file.
 */

#ifndef TALISE_ERROR_TYPES_H_
#define TALISE_ERROR_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "talise_arm_macros.h"

/**
 *  \brief Enum of possible sources of error codes.
 */
typedef enum {
	TAL_ERRSRC_API,     /*!<API Error Src: Error codes defined by taliseErr_t */
	TAL_ERRSRC_ADIHAL,  /*!<ADI HAL Error Src: Error codes defined by adiHalErr_t*/
	TAL_ERRSRC_TALARMCMD,  /*!< TALISE ARM returned error */
	TAL_ERRSRC_TAL_API_GPIO,     /*!< TALISE GPIO returned error */
	TAL_ERRSRC_TALAPIARM,  /*!< TALISE API returned ARM error */
	TAL_ERRSRC_INITCALS,   /*!< TALISE INITCALS returned error */
	TAL_ERRSRC_TAL_API_C0_PCA, /*!< Reserved Error source */
	TAL_ERRSRC_TAL_API_C0_PHMFOVR /*!< Reserved Error source */
} taliseErrSources_t;

/**
 * \brief Enum of unique error codes from the Talise API functions.
 *        Each error condition in the library shall have its own enum value
 *        to ease debug of errors.
 */
typedef enum {
	TAL_ERR_OK=0,
	TAL_ERR_INV_NULL_INIT_PARAM,
	TAL_ERR_WAITFOREVENT_INV_PARM,
	TAL_ERR_CLKPLL_INV_HSDIV,
	TAL_ERR_SETCLKPLL_INV_VCOINDEX,
	TAL_ERR_SETCLKPLL_INV_NDIV,
	TAL_ERR_SETRFPLL_INV_PLLNAME,
	TAL_ERR_SETRFPLL_INITCALS_INPROGRESS,
	TAL_ERR_INV_SCALEDDEVCLK_PARAM,
	TAL_ERR_SETRFPLL_INV_REFCLK,
	TAL_ERR_SETORXGAIN_INV_ORXPROFILE,
	TAL_ERR_SETORXGAIN_INV_CHANNEL,
	TAL_ERR_SETORXGAIN_INV_ORX1GAIN,
	TAL_ERR_SETORXGAIN_INV_ORX2GAIN,
	TAL_ERR_SETTXATTEN_INV_STEPSIZE_PARM,
	TAL_ERR_SETRX1GAIN_INV_GAIN_PARM,
	TAL_ERR_SETRX2GAIN_INV_GAIN_PARM,
	TAL_ERR_SER_INV_M_PARM,
	TAL_ERR_SER_INV_NP_PARM,
	TAL_ERR_SER_INV_L_PARM,
	TAL_ERR_SER_INV_ORX_L_PARM,
	TAL_ERR_SER_INV_ORX_M_PARM,
	TAL_ERR_SER_INV_ORX_NP_PARM,
	TAL_ERR_SER_INV_ORX_LANEEN_PARM,
	TAL_ERR_SER_INV_LANERATE_PARM,
	TAL_ERR_SER_INV_ORX_LANERATE_PARM,
	TAL_ERR_SER_INV_LANEEN_PARM,
	TAL_ERR_SER_INV_AMP_PARM,
	TAL_ERR_SER_INV_PREEMP_PARM,
	TAL_ERR_SER_INV_LANEPN_PARM,
	TAL_ERR_SER_LANE_CONFLICT_PARM,
	TAL_ERR_SER_INV_TXSER_DIV_PARM,
	TAL_ERR_SER_LANE_RATE_CONFLICT_PARM,
	TAL_ERR_SER_INV_ZIF_TO_RIF_DATA_PARM,
	TAL_ERR_SER_INV_DUALBAND_DATA_PARM,
	TAL_ERR_SER_INV_RXFRAMER_SEL,
	TAL_ERR_SER_INV_ORXFRAMER_SEL,
	TAL_ERR_SER_LANERATE_ZERO,
	TAL_ERR_HS_AND_LANE_RATE_NOT_INTEGER_MULT,
	TAL_ERR_DESES_HS_AND_LANE_RATE_NOT_INTEGER_MULT,
	TAL_ERR_DESES_INV_LANE_RATE,
	TAL_ERR_DESES_INV_LANE_RATE_DIV,
	TAL_ERR_DESER_INV_M_PARM,
	TAL_ERR_DESER_INV_NP_PARM,
	TAL_ERR_DESER_INV_L_PARM,
	TAL_ERR_DESER_INV_LANERATE_PARM,
	TAL_ERR_DESER_INV_LANEEN_PARM,
	TAL_ERR_DESER_INV_EQ_PARM,
	TAL_ERR_DESER_INV_LANEPN_PARM,
	TAL_ERR_DESER_LANECONFLICT_PARM,
	TAL_ERR_DESER_M_CONFLICT,
	TAL_ERR_DESER_INV_DEFAB_M,
	TAL_ERR_DESER_NP_CONFLICT,
	TAL_ERR_DESER_INV_DEFRAMERSEL,
	TAL_ERR_DESER_TXPROFILE_INV,
	TAL_ERR_FRAMER_INV_M_PARM,
	TAL_ERR_FRAMER_INV_NP_PARM,
	TAL_ERR_FRAMER_INV_S_PARM,
	TAL_ERR_FRAMER_INV_BANKID_PARM,
	TAL_ERR_FRAMER_INV_LANEID_PARM,
	TAL_ERR_FRAMER_INV_SYNCBIN_PARM,
	TAL_ERR_FRAMER_INV_LMFC_OFFSET_PARAM,
	TAL_ERR_FRAMER_INV_DUALBAND_DATA_PARM,
	TAL_ERR_DEFRAMER_INV_BANKID,
	TAL_ERR_ERR_DEFRAMER_INV_LANEID,
	TAL_ERR_DEFRAMER_INV_LMFC_OFFSET,
	TAL_ERR_DEFRAMER_INV_DEFSEL,
	TAL_ERR_DEFRAMER_INV_TXPROFILE,
	TAL_ERR_DEFRAMER_INV_LANESEN,
	TAL_ERR_DEFRAMER_INV_L,
	TAL_ERR_DEFRAMER_INV_M,
	TAL_ERR_DEFRAMER_INV_NP,
	TAL_ERR_DEFRAMER_INV_F,
	TAL_ERR_DEFRAMER_INV_K,
	TAL_ERR_DEFRAMER_INV_FK,
	TAL_ERR_DEFRAMER_INV_PCLK,
	TAL_ERR_DEFRAMER_INV_PCLKDIV,
	TAL_ERR_RSTDEFRAMER_INV_DEFSEL,
	TAL_ERR_RSTFRAMER_INV_FRAMERSEL,
	TAL_ERR_SETDFRMIRQMASK_INV_DEFRAMERSEL_PARAM,
	TAL_ERR_DEFSTATUS_INV_DEFRAMERSEL_PARAM,
	TAL_ERR_DEFSTATUS_NULL_DEFRAMERSTATUS_PARAM,
	TAL_ERR_GETDFRMIRQMASK_NULL_IRQMASK_PARAM,
	TAL_ERR_GETDFRMIRQMASK_INV_DEFRAMERSELECT_PARAM,
	TAL_ERR_CLRDFRMIRQ_INV_DEFRAMERSEL_PARAM,
	TAL_ERR_GETDFRMIRQSRC_INV_DEFRAMERSEL_PARAM,
	TAL_ERR_GETDFRMIRQSRC_NULL_STATUS_PARAM,
	TAL_ERR_ENDEFSYSREF_INV_DEFRAMERSEL_PARAM,
	TAL_ERR_ENFRAMERSYSREF_INV_FRAMERSEL_PARAM,
	TAL_ERR_FRAMER_INV_TESTDATA_SOURCE_PARAM,
	TAL_ERR_FRAMER_INV_INJECTPOINT_PARAM,
	TAL_ERR_FRAMERSTATUS_INV_FRAMERSEL_PARAM,
	TAL_ERR_FRAMERSTATUS_NULL_FRAMERSTATUS_PARAM,
	TAL_ERR_EN_DEFRAMER_PRBS_INV_PARAM,
	TAL_ERR_READDFRMPRBS_INV_DEFSEL_PARAM,
	TAL_ERR_READDFRMPRBS_NULL_PARAM,
	TAL_ERR_INITARM_INV_ARMCLK_PARAM,
	TAL_ERR_LOADHEX_INVALID_CHKSUM,
	TAL_ERR_LOADBIN_INVALID_BYTECOUNT,
	TAL_ERR_READARMMEM_INV_ADDR_PARM,
	TAL_ERR_WRITEARMMEM_INV_ADDR_PARM,
	TAL_ERR_ARMCMD_INV_OPCODE_PARM,
	TAL_ERR_ARMCMD_INV_NUMBYTES_PARM,
	TAL_ERR_ARMCMDSTATUS_INV_OPCODE_PARM,
	TAL_ERR_JESD204B_ILAS_MISMATCH_NULLPARAM,
	TAL_ERR_JESD204B_ILAS_MISMATCH_NO_ACTIVE_LINK,
	TAL_ERR_JESD204B_ILAS_MISMATCH_SYNC_NOT_DETECTED,
	TAL_ERR_JESD204B_ILAS_MISMATCH_INVALID_DEFRAMER,
	TAL_ERR_TALFINDDFRMRLANECNTERROR_INV_DEFRAMERSEL_PARAM,
	TAL_ERR_TALFINDDFRMRLANECNTERROR_NULL_PARAM,
	TAL_ERR_TALFINDDFRMRLANEERROR_NULL_PARAM,
	TAL_ERR_RXGAINTABLE_INV_CHANNEL,
	TAL_ERR_RXGAINTABLE_INV_PROFILE,
	TAL_ERR_RXGAINTABLE_INV_GAIN_INDEX_RANGE,
	TAL_ERR_ORXGAINTABLE_INV_CHANNEL,
	TAL_ERR_ORXGAINTABLE_INV_PROFILE,
	TAL_ERR_ORXGAINTABLE_INV_GAIN_INDEX_RANGE,
	TAL_ERR_RXFRAMER_INV_FK_PARAM,
	TAL_ERR_RXFRAMER_INV_L_PARAM,
	TAL_ERR_INV_RX_GAIN_MODE_PARM,
	TAL_ERR_UNSUPPORTED_RX_GAIN_MODE_PARM,
	TAL_ERR_INV_AGC_RX_STRUCT_INIT,
	TAL_ERR_INV_AGC_PWR_STRUCT_INIT,
	TAL_ERR_INV_AGC_PWR_LWR0THRSH_PARAM,
	TAL_ERR_INV_AGC_PWR_LWR1THRSH_PARAM,
	TAL_ERR_INV_AGC_PWR_LWR0PWRGAINSTEP_PARAM,
	TAL_ERR_INV_AGC_PWR_LWR1PWRGAINSTEP_PARAM,
	TAL_ERR_INV_AGC_PWR_MSR_DURATION_PARAM,
	TAL_ERR_INV_AGC_PWR_IP3RANGE_PARAM,
	TAL_ERR_INV_AGC_PWR_UPPWR0THRSH_PARAM,
	TAL_ERR_INV_AGC_PWR_UPPWR1THRSH_PARAM,
	TAL_ERR_INV_AGC_PWR_LOGSHIFT_PARAM,
	TAL_ERR_INV_AGC_PWR_UPPWR0GAINSTEP_PARAM,
	TAL_ERR_INV_AGC_PWR_UPPWR1GAINSTEP_PARAM,
	TAL_ERR_INV_AGC_PKK_STRUCT_INIT,
	TAL_ERR_INV_AGC_PKK_HIGHTHRSH_PARAM,
	TAL_ERR_INV_AGC_PKK_LOWGAINMODEHIGHTHRSH_PARAM,
	TAL_ERR_INV_AGC_PKK_LOWGAINHIGHTHRSH_PARAM,
	TAL_ERR_INV_AGC_PKK_LOWGAINTHRSH_PARAM,
	TAL_ERR_INV_AGC_PKK_GAINSTEPATTACK_PARAM,
	TAL_ERR_INV_AGC_PKK_GAINSTEPRECOVERY_PARAM,
	TAL_ERR_INV_AGC_PKK_HB2OVRLD_PARAM,
	TAL_ERR_INV_AGC_PKK_HB2OVRLDDURATION_PARAM,
	TAL_ERR_INV_AGC_PKK_HB2OVRLDTHRSHCNT_PARAM,
	TAL_ERR_INV_AGC_PKK_HB2GAINSTEPRECOVERY_PARAM,
	TAL_ERR_INV_AGC_PKK_HB2GAINSTEP0RECOVERY_PARAM,
	TAL_ERR_INV_AGC_PKK_HB2GAINSTEP1RECOVERY_PARAM,
	TAL_ERR_INV_AGC_PKK_HB2GAINSTEPATTACK_PARAM,
	TAL_ERR_INV_AGC_PKK_HB2OVRLDPWRMODE_PARAM,
	TAL_ERR_INV_AGC_PKK_HB2OVRLDSEL_PARAM,
	TAL_ERR_INV_AGC_PKK_HB2THRSHCFG_PARAM,
	TAL_ERR_INV_AGC_RX_APD_HIGH_LOW_THRESH,
	TAL_ERR_INV_AGC_RX_PEAK_WAIT_TIME_PARM,
	TAL_ERR_INV_AGC_RX_MIN_MAX_GAIN_PARM,
	TAL_ERR_INV_AGC_RX_MIN_GAIN_GRT_THAN_MAX_GAIN_PARM,
	TAL_ERR_INV_AGC_RX_GAIN_UPDATE_TIME_PARM,
	TAL_ERR_INV_AGC_RX1ATTACKDELAY_PARAM,
	TAL_ERR_INV_AGC_RX2ATTACKDELAY_PARAM,
	TAL_ERR_INV_AGC_RX_LOWTHRSHPREVENTGAIN_PARM,
	TAL_ERR_INV_AGC_RX_CHANGEGAINTHRSHHIGH_PARM,
	TAL_ERR_INV_AGC_RX_RESETONRXON_PARM,
	TAL_ERR_INV_AGC_RX_ENSYNCHPULSECAINCNTR_PARM,
	TAL_ERR_INV_AGC_RX_ENIP3OPTTHRSH_PARM,
	TAL_ERR_INV_AGC_RX_ENFASTRECOVERYLOOP_PARM,
	TAL_ERR_INV_AGC_SLOWLOOPDELAY_PARAM,
	TAL_ERR_INV_MINAGCSLOWLOOPDELAY_PARAM,
	TAL_ERR_INV_AGC_CLK_DIV_RATIO_PARM,
	TAL_ERR_INV_AGC_CLK_PARM,
	TAL_ERR_INV_AGC_CLK_RATIO,
	TAL_ERR_INV_AGC_RX_GAIN_UNDERRANGE_UPDATE_TIME_PARM,
	TAL_ERR_INV_AGC_RX_GAIN_UNDERRANGE_MID_INTERVAL_PARM,
	TAL_ERR_INV_AGC_RX_GAIN_UNDERRANGE_HIGH_INTERVAL_PARM,
	TAL_ERR_WAITFOREVENT_TIMEDOUT_CLKPLLCP,
	TAL_ERR_WAITFOREVENT_TIMEDOUT_CLKPLL_LOCK,
	TAL_ERR_WAITFOREVENT_TIMEDOUT_RFPLLCP,
	TAL_ERR_WAITFOREVENT_TIMEDOUT_RFPLL_LOCK,
	TAL_ERR_WAITFOREVENT_TIMEDOUT_AUXPLLCP,
	TAL_ERR_WAITFOREVENT_TIMEDOUT_AUXPLL_LOCK,
	TAL_ERR_WAITFOREVENT_TIMEDOUT_ARMBUSY,
	TAL_ERR_TIMEDOUT_ARMMAILBOXBUSY,
	TAL_ERR_EN_TRACKING_CALS_ARMSTATE_ERROR,
	TAL_ERR_GETPENDINGTRACKINGCALS_NULL_PARAM,
	TAL_ERR_TRACKINGCAL_OUTOFRANGE_PARAM,
	TAL_ERR_PAUSETRACKINGCAL_INV_PARAM,
	TAL_ERR_GETPAUSECALSTATE_NULL_PARAM,
	TAL_ERR_SET_ARMGPIO_PINS_GPIO_IN_USE,
	TAL_ERR_SET_ARMGPIO_PINS_INV_SIGNALID,
	TAL_ERR_SET_ARMGPIO_PINS_INV_GPIOPIN,
	TAL_ERR_SET_RXDATAFRMT_NULL_PARAM,
	TAL_ERR_SET_RXDATAFRMT_FORMATSELECT_INVPARAM,
	TAL_ERR_SET_RXDATAFRMT_TEMPCOMP_INVPARAM,
	TAL_ERR_SET_RXDATAFRMT_ROUNDMODE_INVPARAM,
	TAL_ERR_SET_RXDATAFRMT_FPDATAFRMT_INVPARAM,
	TAL_ERR_SET_RXDATAFRMT_FPENCNAN_INVPARAM,
	TAL_ERR_SET_RXDATAFRMT_FPEXPBITS_INVPARAM,
	TAL_ERR_SET_RXDATAFRMT_FPHIDELEADINGONE_INVPARAM,
	TAL_ERR_SET_RXDATAFRMT_FPRX1ATTEN_INVPARAM,
	TAL_ERR_SET_RXDATAFRMT_FPRX2ATTEN_INVPARAM,
	TAL_ERR_SET_RXDATAFRMT_INTEMBEDDEDBITS_INVPARAM,
	TAL_ERR_SET_RXDATAFRMT_INTSAMPLERESOLUTION_INVPARAM,
	TAL_ERR_SET_RXDATAFRMT_PINSTEPSIZE_INVPARAM,
	TAL_ERR_SET_RXDATAFRMT_RX1GPIOSELECT_INVPARAM,
	TAL_ERR_SET_RXDATAFRMT_RX2GPIOSELECT_INVPARAM,
	TAL_ERR_SET_RXDATAFRMT_RXCHAN_DISABLED,
	TAL_ERR_SET_RXDATAFRMT_EXTERNALLNAGAIN_INVPARAM,
	TAL_ERR_SET_RXDATAFRMT_RX1GPIO_INUSE,
	TAL_ERR_SET_RXDATAFRMT_RX2GPIO_INUSE,
	TAL_ERR_SET_RXDATAFRMT_DATARES_INVPARAM,
	TAL_ERR_SET_RXDATAFRMT_EXTSLICER_RX1GPIO_INVPARAM,
	TAL_ERR_SET_RXDATAFRMT_EXTSLICER_RX2GPIO_INVPARAM,
	TAL_ERR_GET_DATAFORMAT_NULL_PARAM,
	TAL_ERR_GET_SLICERPOS_NULL_PARAM,
	TAL_ERR_INV_RX_DEC_POWER_PARAM,
	TAL_ERR_GETRXDECPOWER_INV_CHANNEL,
	TAL_ERR_GETRXDECPOWER_INV_PROFILE,
	TAL_ERR_INIT_NULLPARAM,
	TAL_ERR_INIT_INV_DEVCLK,
	TAL_ERR_GETRADIOSTATE_NULL_PARAM,
	TAL_ERR_CHECKGETMCS_STATUS_NULL_PARM,
	TAL_ERR_WAIT_INITCALS_ARMERROR,
	TAL_ERR_WAIT_INITCALS_NULL_PARAM,
	TAL_ERR_CHECK_PLL_LOCK_NULL_PARM,
	TAL_ERR_READGPIOSPI_NULL_PARM,
	TAL_ERR_READGPIO3V3SPI_NULL_PARM,
	TAL_ERR_GET_TXFILTEROVRG_NULL_PARM,
	TAL_ERR_PROGRAM_RXGAIN_TABLE_NULL_PARM,
	TAL_ERR_PROGRAM_ORXGAIN_TABLE_NULL_PARM,
	TAL_ERR_PROGRAMFIR_NULL_PARM,
	TAL_ERR_PROGRAMFIR_COEFS_NULL,
	TAL_ERR_READ_DEFRAMERSTATUS_NULL_PARAM,
	TAL_ERR_READ_DEFRAMERPRBS_NULL_PARAM,
	TAL_ERR_ARMCMDSTATUS_NULL_PARM,
	TAL_ERR_PROGRAMFIR_INV_FIRNAME_PARM,
	TAL_ERR_RXFIR_INV_GAIN_PARM,
	TAL_ERR_PROGRAMFIR_INV_NUMTAPS_PARM,
	TALISE_ERR_TXFIR_INV_NUMTAPS_PARM,
	TALISE_ERR_TXFIR_INV_NUMROWS,
	TALISE_ERR_TXFIR_TAPSEXCEEDED,
	TALISE_ERR_RXFIR_INV_DDC,
	TALISE_ERR_RXFIR_INV_NUMTAPS_PARM,
	TALISE_ERR_RXFIR_INV_NUMROWS,
	TALISE_ERR_RXFIR_TAPSEXCEEDED,
	TALISE_ERR_ORXFIR_INV_DDC,
	TALISE_ERR_ORXFIR_INV_NUMTAPS_PARM,
	TALISE_ERR_ORXFIR_INV_NUMROWS,
	TALISE_ERR_ORXFIR_TAPSEXCEEDED,
	TAL_ERR_WAITARMCMDSTATUS_INV_OPCODE,
	TAL_ERR_WAITARMCMDSTATUS_TIMEOUT,
	TAL_ERR_READARMCMDSTATUS_NULL_PARM,
	TAL_ERR_LOADBIN_NULL_PARAM,
	TAL_ERR_GETARMVER_NULL_PARM,
	TAL_ERR_GETARMVER_V2_NULL_PARM,
	TAL_ERR_GETARMVER_V2_INVALID_ARM_NOT_LOADED,
	TAL_ERR_READEVENTSTATUS_INV_PARM,
	TAL_ERR_FRAMER_INV_FRAMERSEL_PARAM,
	TAL_ERR_FRAMER_ERRINJECT_INV_FRAMERSEL_PARAM,
	TAL_ERR_CHECKPLLLOCK_NULL_PARM,
	TAL_ERR_FRAMER_INV_FRAMERSEL_PARM,
	TAL_ERR_SETTXATTEN_INV_TXCHANNEL,
	TAL_ERR_SETTXATTEN_INV_PARM,
	TAL_ERR_RXFRAMER_INV_OUTPUT_RATE,
	TAL_ERR_RXFRAMER_INV_PCLKFREQ,
	TAL_ERR_BBIC_INV_CHN,
	TAL_ERR_INV_GP_INT_MASK_PARM,
	TAL_ERR_INV_GP_INT_MASK_NULL_PARM,
	TAL_ERR_GP_INT_STATUS_NULL_PARAM,
	TAL_ERR_INV_DAC_SAMP_XBAR_CHANNEL_SEL,
	TAL_ERR_INV_ADC_SAMP_XBAR_FRAMER_SEL,
	TAL_ERR_INV_ADC_SAMP_XBAR_SELECT_PARAM,
	TAL_ERR_INV_DAC_SAMP_XBAR_SELECT_PARAM,
	TAL_ERR_GETRFPLL_INV_PLLNAME,
	TAL_ERR_GET_PLLFREQ_INV_REFCLKDIV,
	TAL_ERR_GET_PLLFREQ_INV_HSDIV,
	TAL_ERR_GETRFPLL_NULLPARAM,
	TAL_ERR_FRAMER_A_AND_B_INV_M_PARM,
	TAL_ERR_SETRXGAIN_RXPROFILE_INVALID,
	TAL_ERR_SETRXGAIN_INV_CHANNEL,
	TAL_ERR_INIT_CALS_COMPLETED_NULL_PTR,
	TAL_ERR_CHKINITCALS_NULL_PTR,
	TAL_ERR_INIT_CALS_LASTRUN_NULL_PTR,
	TAL_ERR_GETENABLED_TRACK_CALS_NULL_PTR,
	TAL_ERR_INIT_CALS_MIN_NULL_PTR,
	TAL_ERR_INIT_ERR_CAL_NULL_PTR,
	TAL_ERR_INIT_ERR_CODE_NULL_PTR,
	TAL_ERR_READARMCFG_ARMERRFLAG,
	TAL_ERR_GETRXGAIN_INV_RXPROFILE,
	TAL_ERR_GETRXGAIN_INV_CHANNEL,
	TAL_ERR_GETRXGAIN_GAIN_RANGE_EXCEEDED,
	TAL_ERR_GETOBSRXGAIN_INV_ORXPROFILE,
	TAL_ERR_GETOBSRXGAIN_INV_CHANNEL,
	TAL_ERR_GETOBSRXGAIN_GAIN_RANGE_EXCEEDED,
	TAL_ERR_SETUPDUALBANDRXAGC_GAIN_RANGE_MISMATCH,
	TAL_ERR_SETUPDUALBANDRXAGC_GAIN_OUT_OF_RANGE,
	TAL_ERR_VERIFYBIN_CHECKSUM_TIMEOUT,
	TAL_ERR_ARMCMDSTATUS_ARMERROR,
	TAL_ERR_VERRXPFILE_INV_IQRATE,
	TAL_ERR_VERRXPFILE_INV_RFBW,
	TAL_ERR_VERRXPFILE_INV_RHB1,
	TAL_ERR_VERRXPFILE_INV_DEC5,
	TAL_ERR_VERRXPFILE_INV_FIR,
	TAL_ERR_VERRXPFILE_INV_COEF,
	TAL_ERR_VERRXPFILE_INV_DDC,
	TAL_ERR_VERORXPFILE_INV_IQRATE,
	TAL_ERR_VERORXPFILE_INV_RFBW,
	TAL_ERR_VERORXPFILE_INV_RHB1,
	TAL_ERR_VERORXPFILE_INV_DEC5,
	TAL_ERR_VERORXPFILE_INV_FIR,
	TAL_ERR_VERORXPFILE_INV_COEF,
	TAL_ERR_VERORXPFILE_INV_DDC,
	TAL_ERR_VERTXPFILE_INV_DISONPLLUNLOCK,
	TAL_ERR_VERTXPFILE_INV_IQRATE,
	TAL_ERR_VERTXPFILE_INV_RFBW,
	TAL_ERR_VERTXPFILE_INV_THB1,
	TAL_ERR_VERTXPFILE_INV_THB2,
	TAL_ERR_VERTXPFILE_INV_THB3,
	TAL_ERR_VERTXPFILE_INV_INT5,
	TAL_ERR_VERTXPFILE_INV_HBMUTEX,
	TAL_ERR_VERTXPFILE_INV_FIRIPL,
	TAL_ERR_VERTXPFILE_INV_COEF,
	TAL_ERR_VERTXPFILE_INV_DACDIV,
	TAL_ERR_VERPFILE_INV_RFPLLMCSMODE,
	TAL_ERR_VERPFILE_TXHSCLK,
	TAL_ERR_VERPFILE_RXHSCLK,
	TAL_ERR_VERPFILE_ORXHSCLK,
	TAL_ERR_SETSPI_INV_CMOS_DRV_STR,
	TAL_ERR_SETCLKPLL_INV_TXATTENDIV,
	TAL_ERR_SETTXTOORXMAP_INV_ORX1_MAP,
	TAL_ERR_SETTXTOORXMAP_INV_ORX2_MAP,
	TAL_ERR_GETRXTXENABLE_NULLPARAM,
	TAL_ERR_SETRXTXENABLE_INVCHANNEL,
	TAL_ERR_GETTXATTEN_NULL_PARM,
	TAL_ERR_GETTXATTEN_INV_TXCHANNEL,
	TAL_ERR_INV_RADIO_CTL_MASK_PARM,
	TAL_ERR_GETPINMODE_NULLPARAM,
	TAL_ERR_SET_ARMGPIO_NULLPARAM,
	TAL_ERR_GETTEMPERATURE_NULLPARAM,
	TAL_ERR_GETTXLOLSTATUS_NULL_PARAM,
	TAL_ERR_GETTXLOLSTATUS_INV_CHANNEL_PARM,
	TAL_ERR_GETTXQECSTATUS_NULL_PARAM,
	TAL_ERR_GETTXQECSTATUS_INV_CHANNEL_PARM,
	TAL_ERR_GETRXQECSTATUS_NULL_PARAM,
	TAL_ERR_GETRXQECSTATUS_INV_CHANNEL_PARM,
	TAL_ERR_GETORXQECSTATUS_NULL_PARAM,
	TAL_ERR_GETORXQECSTATUS_INV_CHANNEL_PARM,
	TAL_ERR_GETRXHD2STATUS_NULL_PARAM,
	TAL_ERR_GETRXHD2STATUS_INV_CHANNEL_PARM,
	TAL_ERR_VERIFYSPI_READ_LOW_ADDR_ERROR,
	TAL_ERR_VERIFYSPI_WRITE_LOW_ADDR_ERROR,
	TAL_ERR_VERIFYSPI_READ_HIGH_ADDR_ERROR,
	TAL_ERR_VERIFYSPI_WRITE_HIGH_ADDR_ERROR,
	TAL_ERR_GETAPIVERSION_NULLPARAM,
	TAL_ERR_INV_DAC_FULLSCALE_PARM,
	TAL_ERR_RESET_TXLOL_INV_CHANNEL_PARM,
	TAL_ERR_RESET_TXLOL_ARMSTATE_ERROR,
	TAL_ERR_SETHD2CFG_NULL_PARAM,
	TAL_ERR_SETHD2CFG_ARMSTATE_ERROR,
	TAL_ERR_GETHD2CFG_NULL_PARAM,
	TAL_ERR_GETHD2CFG_ARMSTATE_ERROR,
	TAL_ERR_SET_SPI2_ENABLE_INVALID_TX_ATTEN_SEL,
	TAL_ERR_SET_SPI2_ENABLE_GPIO_IN_USE,
	TAL_ERR_SETRXMGCPINCTRL_INV_RX1_INC_PIN,
	TAL_ERR_SETRXMGCPINCTRL_INV_RX1_DEC_PIN,
	TAL_ERR_SETRXMGCPINCTRL_INV_RX2_INC_PIN,
	TAL_ERR_SETRXMGCPINCTRL_INV_RX2_DEC_PIN,
	TAL_ERR_SETRXMGCPINCTRL_INV_CHANNEL,
	TAL_ERR_SETRXMGCPINCTRL_INV_INC_STEP,
	TAL_ERR_SETRXMGCPINCTRL_INV_DEC_STEP,
	TAL_ERR_SETRXMGCPINCTRL_RX1_GPIO_IN_USE,
	TAL_ERR_SETRXMGCPINCTRL_RX2_GPIO_IN_USE,
	TAL_ERR_GETRXMGCPINCTRL_INV_CHANNEL,
	TAL_ERR_GETRXMGCPINCTRL_NULL_PARAM,
	TAL_ERR_SETRFPLL_LOOPFILTER_INV_LOOPBANDWIDTH,
	TAL_ERR_SETRFPLL_LOOPFILTER_INV_STABILITY,
	TAL_ERR_SETRFPLL_LOOPFILTER_INV_PLLSEL,
	TAL_ERR_GETRFPLL_LOOPFILTER_NULLPARAM,
	TAL_ERR_GETRFPLL_LOOPFILTER_INV_PLLSEL,
	TAL_ERR_GETDEVICEREV_NULLPARAM,
	TAL_ERR_GETPRODUCTID_NULLPARAM,
	TAL_ERR_ENABLETXNCO_INV_PROFILE,
	TAL_ERR_ENABLETXNCO_NULL_PARM,
	TAL_ERR_ENABLETXNCO_INV_TX1_FREQ,
	TAL_ERR_ENABLETXNCO_INV_TX2_FREQ,
	TAL_ERR_SETTXATTENCTRLPIN_NULL_PARAM,
	TAL_ERR_SETTXATTENCTRLPIN_TX1_GPIO_IN_USE,
	TAL_ERR_SETTXATTENCTRLPIN_TX2_GPIO_IN_USE,
	TAL_ERR_SETTXATTENCTRLPIN_INV_PARM,
	TAL_ERR_SETTXATTENCTRLPIN_INV_TX1_INC_PIN,
	TAL_ERR_SETTXATTENCTRLPIN_INV_TX1_DEC_PIN,
	TAL_ERR_SETTXATTENCTRLPIN_INV_TX2_INC_PIN,
	TAL_ERR_SETTXATTENCTRLPIN_INV_TX2_DEC_PIN,
	TAL_ERR_SETTXATTENCTRLPIN_INV_CHANNEL,
	TAL_ERR_GETTXATTENCTRLPIN_INV_CHANNEL,
	TAL_ERR_GETTXATTENCTRLPIN_NULL_PARAM,
	TAL_ERR_SETUPDUALBANDRX1AGC_GPIO3P3_IN_USE,
	TAL_ERR_SETUPDUALBANDRX2AGC_GPIO3P3_IN_USE,
	TAL_ERR_SETUPDUALBANDRXAGC_INV_CHANNEL,
	TAL_ERR_SETUPDUALBANDRXAGC_NULL_PARAM,
	TAL_ERR_SETUPDUALBANDRXAGC_INV_PWRMARGIN,
	TAL_ERR_SETUPDUALBANDRXAGC_INV_DECPWR,
	TAL_ERR_GETDUALBANDLNA_INV_CHANNEL,
	TAL_ERR_GETDUALBANDLNA_NULL_PARAM,
	TAL_ERR_INITIALIZE_DDC_INV_TOTAL_M_2OR4,
	TAL_ERR_INITIALIZE_DDC_INV_TOTAL_M_4OR8,
	TAL_ERR_INITIALIZE_DDC_NULL_PARAM,
	TAL_ERR_RXNCOFTW_INVNCO,
	TAL_ERR_INITIALIZE_DDC_INV_RXDDCMODE,
	TAL_ERR_INITIALIZE_DDC_INV_DEC_AT_PFIR,
	TAL_ERR_SETDUALBANDSETTINGS_INV_CENTER_FREQ,
	TAL_ERR_SETDUALBANDSETTINGS_INV_BAND_SEP,
	TAL_ERR_SETDUALBANDSETTINGS_INV_IN_UPPER_FREQ,
	TAL_ERR_SETDUALBANDSETTINGS_INV_IN_LOWER_FREQ,
	TAL_ERR_SETDUALBANDSETTINGS_INV_OUT_UPPER_FREQ,
	TAL_ERR_SETDUALBANDSETTINGS_INV_OUT_LOWER_FREQ,
	TAL_ERR_SETDUALBANDSETTINGS_OUT_OVERLAP,
	TAL_ERR_SETDUALBANDSETTINGS_FTW_OVRG,
	TAL_ERR_DUALBAND_LNA_TABLE_INV_PROFILE,
	TAL_ERR_DUALBAND_LNA_TABLE_INV_INDEX,
	TAL_ERR_DUALBAND_LNA_TABLE_INV_CHANNEL,
	TAL_ERR_DUALBAND_LNA_TABLE_NULL_PARM,
	TAL_ERR_SETUPNCOSHIFTER_INV_PFIR_CORNER,
	TAL_ERR_SETUPNCOSHIFTER_INV_DDCHB_CORNER,
	TAL_ERR_SETUPNCOSHIFTER_INV_NCO2SHIFT,
	TAL_ERR_SETUPNCOSHIFTER_FTW_OVRG,
	TAL_ERR_INV_DEFA_SLEWRATE,
	TAL_ERR_INV_DEFB_SLEWRATE,
	TAL_ERR_GETTXSAMPLEPWR_NULL_PARAM,
	TAL_ERR_GETTXSAMPLEPWR_INV_TXREADCHAN,
	TAL_ERR_SETPAPRO_NULL_PARAM,
	TAL_ERR_SETPAPRO_INV_AVGDURATION,
	TAL_ERR_SETPAPROT_INV_PEAKCNT,
	TAL_ERR_SETPAPROT_INV_TXATTENSTEP,
	TAL_ERR_SETPAPROT_INV_TX1THRESH,
	TAL_ERR_SETPAPROT_INV_TX2THRESH,
	TAL_ERR_SETPAPROT_INV_TX1PEAKTHRESH,
	TAL_ERR_SETPAPROT_INV_TX2PEAKTHRESH,
	TAL_ERR_GETPAERRFLAGS_NULL_PARAM,
	TAL_ERR_GETPAPRO_NULL_PARAM,
	TAL_ERR_DAC_FULLSCALE_INVARMSTATE,
	TAL_ERR_DEFSTATUS_INV_COUNTERERRTHRESHOLD_PARAM,
	TAL_ERR_RFPLLFREQ_TX_OUT_OF_RANGE,
	TAL_ERR_RFPLLFREQ_RX_OUT_OF_RANGE,
	TAL_ERR_RFPLLFREQ_ORX_OUT_OF_RANGE,
	TAL_ERR_READEVENTSTATUS_NULL_PARM,
	TAL_ERR_INV_SIREV,
	TAL_ERR_REGISTER_ERRORMSG_C0,
	TAL_ERR_SETEXTWORDCTRLGPIO_INV_CHANNEL,
	TAL_ERR_SETEXTWORDCTRLGPIO_UNINITIALIZED_RX1,
	TAL_ERR_SETEXTWORDCTRLGPIO_UNINITIALIZED_RX2,
	TAL_ERR_SETEXTWORDCTRLGPIO_GPIO_IN_USE_RX1,
	TAL_ERR_SETEXTWORDCTRLGPIO_GPIO_IN_USE_RX2,
	TAL_ERR_FRAMERSYSREFTOGGLE_INV_FRAMERSEL_PARAM,
	TAL_ERR_SETORXLOSRC_INVALIDPARAM,
	TAL_ERR_GETORXLOSRC_NULLPARAM,
	TAL_ERR_SETORXLOSRC_TIMEDOUT_ARMMAILBOXBUSY,
	TAL_ERR_SETORXLOCFG_NULL_PARAM,
	TAL_ERR_SETORXLOCFG_INVALIDPARAM,
	TAL_ERR_SETORXLOCFG_INVALID_ARMSTATE,
	TAL_ERR_SETORXLOCFG_GPIOUSED,
	TAL_ERR_GETORXLOCFG_NULL_PARAM,
	TAL_ERR_GETORXLOCFG_INVALID_ARMSTATE,
	TAL_ERR_SETFHMCONFIG_NULL_PARAM,
	TAL_ERR_SETFHMCONFIG_INV_FHMGPIOPIN,
	TAL_ERR_SETFHMCONFIG_INV_FHMCONFIG_FHM_MIN_FREQ,
	TAL_ERR_SETFHMCONFIG_INV_FHMCONFIG_FHM_MAX_FREQ,
	TAL_ERR_SETFHMCONFIG_FHMGPIOPIN_IN_USE,
	TAL_ERR_GETFHMCONFIG_NULL_PARAM,
	TAL_ERR_SETFHMMODE_NULL_PARAM,
	TAL_ERR_SETFHMMODE_INV_FHM_INIT_FREQ,
	TAL_ERR_SETFHMMODE_INV_FHM_TRIGGER_MODE,
	TAL_ERR_SETFHMMODE_INV_FHM_EXIT_MODE,
	TAL_ERR_GETFHMMODE_NULL_PARAM,
	TAL_ERR_SETFHMHOP_INV_FHM_FREQ,
	TAL_ERR_GETFHMSTS_NULL_PARAM,
	TAL_ERR_SETDCMSHIFT_INV_CH_PARAM,
	TAL_ERR_SETDCMSHIFT_INV_MSHIFT_PARAM,
	TAL_ERR_GETDCMSHIFT_INV_CH_PARAM,
	TAL_ERR_GETDCMSHIFT_NULL_MSHIFT_PARAM,
	TAL_ERR_SETEXTLOOUT_INV_DIV_PARAM,
	TAL_ERR_SETEXTLOOUT_LO_IN_ENABLED,
	TAL_ERR_GETEXTLOOUT_NULL_PARAM,
	TAL_ERR_DIG_DC_OFFSET_INV_ENABLE_MASK,
	TAL_ERR_DIG_DC_OFFSET_NULL_ENABLE_MASK,
	TAL_ERR_NUMBER_OF_ERRORS /* Keep this ENUM last as a reference to the total number of error enum values */
} taliseErr_t;

/**
 *  \brief Private Enum to list of available error handlers
 */
typedef enum {
	TAL_ERRHDL_HAL_WAIT, /*!<API Error handler for HAL wait/delay function errors */
	TAL_ERRHDL_HAL_LOG,  /*!<API Error handler for HAL log function error */
	TAL_ERRHDL_HAL_SPI,  /*!<API Error handler for HAL SPI function errors */
	TAL_ERRHDL_HAL_GPIO, /*!<API Error handler for HAL GPIO function errors */
	TAL_ERRHDL_INVALID_PARAM, /*!<API Error handler invalid parameter errors */
	TAL_ERRHDL_API_FAIL,  /*!<API Error handler API functional errors */
	TAL_ERRHDL_APIARM_ERR,   /*!< Talise API layer ARM error handler */
	TAL_ERRHDL_ARM_CMD_ERR, /*!< Talise sendArmCommand error handler */
	TAL_ERRHDL_ARM_INITCALS_ERR, /*!< Talise init calibration error handler */
	TAL_ERRHDL_API_GPIO,   /*!< Talise GPIO error handler */
	TAL_ERRHDL_API_C0_PCA,   /*!< reserved error handler */
	TAL_ERRHDL_API_C0_PHMFOVR  /*!< reserved error handler */
} taliseErrHdls_t;

/**
 *  \brief Enumerated list of Talise Recovery Actions used as return values
 *         APIs.
 */
typedef enum {
	TALACT_NO_ACTION = 0,            /*!< API OK - NO ACTION REQUIRED */
	TALACT_WARN_RESET_LOG,           /*!< API OK - LOG Not working */
	TALACT_WARN_RERUN_TRCK_CAL,      /*!< API NG - RESET  SPEC TRACK CALS */
	TALACT_WARN_RESET_GPIO,          /*!< API OK - GPIO Not working */
	TALACT_ERR_CHECK_TIMER,          /*!< API OK - timer not working */
	TALACT_ERR_RESET_ARM,            /*!< API NG - RESET ARM ONLY */
	TALACT_ERR_RERUN_INIT_CALS,      /*!< API NG - RESET INIT CAL SEQ */
	TALACT_ERR_RESET_SPI,            /*!< API NG - SPI Not Working */
	TALACT_ERR_RESET_GPIO,           /*!< API NG - GPIO Not working */
	TALACT_ERR_CHECK_PARAM,          /*!< API OK - INVALID PARAM */
	TALACT_ERR_RESET_FULL,           /*!< API NG - FULL RESET REQUIRED */
	TALACT_ERR_RESET_JESD204FRAMERA, /*!< API NG - RESET the JESD204 FRAMER A */
	TALACT_ERR_RESET_JESD204FRAMERB, /*!< API NG - RESET the JESD204 FRAMER B */
	TALACT_ERR_RESET_JESD204DEFRAMERA, /*!< API NG - RESET the JESD204 DEFRAMER A */
	TALACT_ERR_RESET_JESD204DEFRAMERB, /*!< API NG - RESET the JESD204 DEFRAMER B */
	TALACT_ERR_BBIC_LOG_ERROR,           /*!< API NG - USER Should log this error and decide recovery action */
	TALACT_ERR_REDUCE_TXSAMPLE_PWR     /*!< API NG - RESET the TX Sample power for the Channel specified */
} talRecoveryActions_t;

#ifdef __cplusplus
}
#endif

#endif /* TALISE_ERROR_TYPES_H_ */
