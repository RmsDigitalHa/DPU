/*
 * rf_control.h
 *
 *  Created on: 2023. 12. 30.
 *      Author: RMS_Digital
 */

#ifndef INC_RF_CONTROL_H_
#define INC_RF_CONTROL_H_


#define RCFM 	0U
#define RCRM	1U


#define 	G_ATTEN_TABLE_SIZE	167U


typedef struct
{
	uint64_t		u64StartFreq;
	uint8_t			u8AMP1_GAIN_ATTEN;
	uint8_t			u8AMP2_GAIN_ATTEN;
	uint8_t		 	u8BYPASS_GAIN_ATTEN;
	uint8_t		 	u8SYSTEM_ATTEN;
}
typTableATTEN;


static int GetRFPathStatus(void);
void GetStatusPBIT(void);
void GetStatusIBIT(void);
void SetGainAtten(const uint64_t Freq);
static typTableATTEN GetAttenValue(const uint64_t TargetFreq);

#endif /* INC_RF_CONTROL_H_ */
