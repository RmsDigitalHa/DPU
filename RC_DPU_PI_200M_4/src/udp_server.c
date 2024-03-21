/*
 * udp_server.c
 *
 *  Created on: 2024. 1. 24.
 *      Author: Lee Jeho
 */

#include "lwip/ip_addr.h"
#include "lwip/err.h"
#include "lwip/udp.h"
#include "lwip/inet.h"

#include "udp_server.h"
#include "icd_parser.h"
#include "rf_control.h"
#include "rcfm_control.h"
#include "rcrm_control.h"
#include "init.h"
#include "user_func.h"

#include "xstatus.h"

//#define DEFAULT_IP_ADDRESS	"192.168.1.10"
//#define DEFAULT_IP_MASK		"255.255.255.0"
//#define DEFAULT_GW_ADDRESS	"192.168.1.1"
#define DEFAULT_IP_ADDRESS	"192.168.10.71"
#define DEFAULT_IP_MASK		"255.255.255.0"
#define DEFAULT_GW_ADDRESS	"192.168.10.1"

//#define UDP_CONN_PORT 10001
#define UDP_CONN_PORT 5051

#define NETWORK_BUFSIZE	64

static struct udp_pcb *udp_pcb;
struct netif server_netif;

uint8_t recv_buf_udp[NETWORK_BUFSIZE], reply_buf_udp[NETWORK_BUFSIZE];
uint8_t SendDone;
// Client IP, Port (RC_Controller)
ip_addr_t client_ip;
uint16_t client_port;

// DMA Code Variable
extern uint32_t *AddrSpecHeader;
extern uint32_t *AddrSpecPrevHeader;
extern uint32_t *AddrSpecCurHeader;
extern uint8_t DataReady;


static void PrintIp(char *msg, ip_addr_t *ip)
{
	print(msg);
	xil_printf("%d.%d.%d.%d\r\n", ip4_addr1(ip), ip4_addr2(ip),
			ip4_addr3(ip), ip4_addr4(ip));
}

static void PrintIpSettings(ip_addr_t *ip, ip_addr_t *mask, ip_addr_t *gw)
{
	PrintIp("Board IP:       ", ip);
	PrintIp("Netmask :       ", mask);
	PrintIp("Gateway :       ", gw);
}

static void AssignDefaultIp(ip_addr_t *ip, ip_addr_t *mask, ip_addr_t *gw)
{
	int err;

	xil_printf("Configuring default IP %s \r\n", DEFAULT_IP_ADDRESS);

	err = inet_aton(DEFAULT_IP_ADDRESS, ip);
	if (!err) {
		xil_printf("Invalid default IP address: %d\r\n", err);
	}
	err = inet_aton(DEFAULT_IP_MASK, mask);
	if (!err) {
		xil_printf("Invalid default IP MASK: %d\r\n", err);
	}
	err = inet_aton(DEFAULT_GW_ADDRESS, gw);
	if (!err) {
		xil_printf("Invalid default gateway address: %d\r\n", err);
	}
}

static void RecvCallback(void *arg, struct udp_pcb *tpcb,
		struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
	ICD_HEADER recv_icd_header;
	err_t error;
	uint16_t log_val;

	struct pbuf *send_packet;

	send_packet = pbuf_alloc(PBUF_TRANSPORT, UDP_SEND_BUFSIZE, PBUF_POOL);
	if(!send_packet){
		xil_printf("error allocating pbuf to send\r\n");
		return;
	}

	memcpy(recv_buf_udp, p->payload, p->len);
	memcpy(reply_buf_udp, recv_buf_udp, p->len);

	recv_icd_header = ParserTCP(recv_buf_udp, p->len);
	SwapOPCODE(reply_buf_udp);		//Source Code <-> Destination Code

	printf("== UDP Recv\n");
	printf("== IP : %s, port : %d, cmd_code : 0x%X\n", inet_ntoa(addr), port, recv_icd_header.CMD_CODE);

	if(recv_icd_header.SRC_CODE == 0xE1U && recv_icd_header.DEST_CODE == 0x51U){
		/////////////////////// DPU_CTRL //////////////////////////
		if(recv_icd_header.CMD_CODE == 0x0010U){				//Set Center Frequency
			reply_buf_udp[4] = 0x09;
			*((uint64_t *)&reply_buf_udp[8]) = (uint64_t)(DPU_STATUS.CenterFreq);
			reply_buf_udp[16] = 0x01;

			//RCV Packet
			memcpy(send_packet->payload, reply_buf_udp, p->len + 1);
			send_packet->len = p -> len + 1;
			send_packet->tot_len = p -> len + 1;
		}
		else if(recv_icd_header.CMD_CODE == 0x0020U){		//Set BW
			reply_buf_udp[4] = 0x02;
			reply_buf_udp[8] = DPU_STATUS.ParmBw;
			reply_buf_udp[9] = 0x01;

			//RCV Packet
			memcpy(send_packet->payload, reply_buf_udp, p->len + 1);
			send_packet->len = p -> len + 1;
			send_packet->tot_len = p -> len + 1;
		}
		else if(recv_icd_header.CMD_CODE == 0x0030U){		//Set RBW
			reply_buf_udp[4] = 0x02;
			reply_buf_udp[8] = DPU_STATUS.ParmRbw;
			reply_buf_udp[9] = 0x00;

			//RCV Packet
			memcpy(send_packet->payload, reply_buf_udp, p->len + 1);
			send_packet->len = p -> len + 1;
			send_packet->tot_len = p -> len + 1;
		}
		else if(recv_icd_header.CMD_CODE == 0x0040U){		//Spectrum Transfer Start
			client_ip = *(addr);
			client_port = port;
		}
		else if(recv_icd_header.CMD_CODE == 0x0050U){		//Spectrum Transfer Stop
			reply_buf_udp[4] = 0x02;
			reply_buf_udp[8] = DPU_STATUS.START;
			reply_buf_udp[9] = 0x00;

			//RCV Packet
			memcpy(send_packet->payload, reply_buf_udp, p->len + 1);
			send_packet->len = p -> len + 1;
			send_packet->tot_len = p -> len + 1;
		}
		else if(recv_icd_header.CMD_CODE == 0x0060U){		//Get BIT Status
			if(BIT_STATUS.BIT_SET == 0U){

			}
			else{
				if(reply_buf_udp[8] == 1U){			//PBIT
					reply_buf_udp[2] = 0x61;
					reply_buf_udp[4] = 0x13;
					reply_buf_udp[9] = PBIT_STATUS.LOCK_ADCLK;
					reply_buf_udp[10] = PBIT_STATUS.LOCK_BIT;
					reply_buf_udp[11] = RCFM_LNA;				//Init Val = BYPASS, LNA Mode for check LNA status
					reply_buf_udp[12] = PBIT_STATUS.LNA1;
					reply_buf_udp[13] = PBIT_STATUS.REF_SIG;
					reply_buf_udp[14] = PBIT_STATUS.RCFM_PWR;
					reply_buf_udp[15] = PBIT_STATUS.RCFM_INSERT;
					reply_buf_udp[16] = PBIT_STATUS.RCFM_TMP;
					reply_buf_udp[17] = RCRM_LNA02;				//Init Val = BYPASS, LNA Mode for check LNA status
					reply_buf_udp[18] = PBIT_STATUS.LNA2;
					reply_buf_udp[19] = PBIT_STATUS.LNA3;
					reply_buf_udp[20] = PBIT_STATUS.RCRM_PWR;
					reply_buf_udp[21] = PBIT_STATUS.RCRM_INSERT;
					reply_buf_udp[22] = PBIT_STATUS.RCRM_TMP;
					reply_buf_udp[23] = PBIT_STATUS.DONE_FPGA;
					reply_buf_udp[24] = PBIT_STATUS.DONE_ADC;
					reply_buf_udp[25] = PBIT_STATUS.DONE_DDR;
					reply_buf_udp[26] = PBIT_STATUS.RF_PATH;

					//RCV Packet
					memcpy(send_packet->payload, reply_buf_udp, p->len + 18);
					send_packet->len = p -> len + 18;
					send_packet->tot_len = p -> len + 18;
				}
				else if(reply_buf_udp[8] == 2U){		//CBIT
					GetStatusIBIT();

					reply_buf_udp[2] = 0x62;
					reply_buf_udp[4] = 0x0d;
					reply_buf_udp[9] = BIT_STATUS.LOCK_ADCLK;
					reply_buf_udp[10] = BIT_STATUS.REF_SIG;
					reply_buf_udp[11] = BIT_STATUS.RCFM_PWR;
					reply_buf_udp[12] = BIT_STATUS.RCFM_INSERT;
					reply_buf_udp[13] = BIT_STATUS.RCFM_TMP;
					reply_buf_udp[14] = BIT_STATUS.LNA3;
					reply_buf_udp[15] = BIT_STATUS.RCRM_PWR;
					reply_buf_udp[16] = BIT_STATUS.RCRM_INSERT;
					reply_buf_udp[17] = BIT_STATUS.RCRM_TMP;
					reply_buf_udp[18] = BIT_STATUS.DONE_FPGA;
					reply_buf_udp[19] = BIT_STATUS.DONE_ADC;
					reply_buf_udp[20] = BIT_STATUS.DONE_DDR;

					//RCV Packet
					memcpy(send_packet->payload, reply_buf_udp, p->len + 12);
					send_packet->len = p -> len + 12;
					send_packet->tot_len = p -> len + 12;
				}
				else if(reply_buf_udp[8] == 3U){		//IBIT
					GetStatusIBIT();

					reply_buf_udp[2] = 0x63;
					reply_buf_udp[4] = 0x13;
					reply_buf_udp[9] = BIT_STATUS.LOCK_ADCLK;
					reply_buf_udp[10] = BIT_STATUS.LOCK_BIT;
					reply_buf_udp[11] = RCFM_LNA;
					reply_buf_udp[12] = BIT_STATUS.LNA1;
					reply_buf_udp[13] = BIT_STATUS.REF_SIG;
					reply_buf_udp[14] = BIT_STATUS.RCFM_PWR;
					reply_buf_udp[15] = BIT_STATUS.RCFM_INSERT;
					reply_buf_udp[16] = BIT_STATUS.RCFM_TMP;
					reply_buf_udp[17] = RCRM_LNA02;
					reply_buf_udp[18] = BIT_STATUS.LNA2;
					reply_buf_udp[19] = BIT_STATUS.LNA3;
					reply_buf_udp[20] = BIT_STATUS.RCRM_PWR;
					reply_buf_udp[21] = BIT_STATUS.RCRM_INSERT;
					reply_buf_udp[22] = BIT_STATUS.RCRM_TMP;
					reply_buf_udp[23] = BIT_STATUS.DONE_FPGA;
					reply_buf_udp[24] = BIT_STATUS.DONE_ADC;
					reply_buf_udp[25] = BIT_STATUS.DONE_DDR;
					reply_buf_udp[26] = BIT_STATUS.RF_PATH;

					//RCV Packet
					memcpy(send_packet->payload, reply_buf_udp, p->len + 18);
					send_packet->len = p -> len + 18;
					send_packet->tot_len = p -> len + 18;
				}

				BIT_STATUS.BIT_SET = 0;
			}
		}
		/////////////////////// RF_CTRL //////////////////////////
		else if(recv_icd_header.CMD_CODE == 0x0100U){		//Set RC_RCV Freq
			if(reply_buf_udp[8] == SET){

			}
			else if(reply_buf_udp[8] == GET){
				reply_buf_udp[4] = 0x09;
				*((uint64_t *)&reply_buf_udp[9]) = (uint64_t)(rcrm_status.rcrm_freq_hz - FREQ_OFFSET);

				//RCV Packet
				memcpy(send_packet->payload, reply_buf_udp, p->len);
				send_packet->len = p -> len;
				send_packet->tot_len = p -> len;
			}
		}
		else if(recv_icd_header.CMD_CODE == 0x0110U){		//Set RF Filter Path
			if(reply_buf_udp[8] == SET){

			}
			else if(reply_buf_udp[8] == GET){
				if(reply_buf_udp[9] == 0U){			//BPF
					reply_buf_udp[10] = rcrm_status.rcrm_bpf_bank;
				}
				else if(reply_buf_udp[9] == 1U){		//LPF
					reply_buf_udp[10] = rcrm_status.rcrm_lpf_bank;
				}
				reply_buf_udp[4] = 0x03;

				//RCV Packet
				memcpy(send_packet->payload, reply_buf_udp, p->len);
				send_packet->len = p -> len;
				send_packet->tot_len = p -> len;
			}
		}
		else if(recv_icd_header.CMD_CODE == 0x0120U){		//Set RF AMP Path
			if(reply_buf_udp[8] == SET){

			}
			else if(reply_buf_udp[8] == GET){
				if(reply_buf_udp[9] == 0U){
					reply_buf_udp[10] = rcfm_status.rcfm_amp_mode1;

				}
				else if(reply_buf_udp[9] == 1U){
					reply_buf_udp[10] = rcrm_status.rcrm_amp_mode2;
				}
				reply_buf_udp[4] = 0x03;

				//RCV Packet
				memcpy(send_packet->payload, reply_buf_udp, p->len);
				send_packet->len = p -> len;
				send_packet->tot_len = p -> len;
			}
		}
		else if(recv_icd_header.CMD_CODE == 0x0130U){		//Set RF Atten
			if(reply_buf_udp[8] == SET){

			}
			else if(reply_buf_udp[8] == GET){
				if(reply_buf_udp[9] == 0U){			//SYS_ATTEN
					reply_buf_udp[10] = (rcrm_status.rcrm_sys_att);
				}
				else if(reply_buf_udp[9] == 1U){		//GAIN_ATTEN
					reply_buf_udp[10] = (rcrm_status.rcrm_gain_att);
				}
				reply_buf_udp[4] = 0x03;

				//RCV Packet
				memcpy(send_packet->payload, reply_buf_udp, p->len);
				send_packet->len = p -> len;
				send_packet->tot_len = p -> len;
			}
		}
		else if(recv_icd_header.CMD_CODE == 0x0140U){		//Set RF_RCV Path
			if(reply_buf_udp[8] == SET){

			}
			else if(reply_buf_udp[8] == GET){
				reply_buf_udp[4] = 0x02;
				reply_buf_udp[9] =  rcfm_status.rcfm_rf_select;

				//RCV Packet
				memcpy(send_packet->payload, reply_buf_udp, p->len);
				send_packet->len = p -> len;
				send_packet->tot_len = p -> len;
			}
		}
		else if(recv_icd_header.CMD_CODE == 0x0150U){		//Set RF BIT EN
			if(reply_buf_udp[8] == SET){

			}
			else if(reply_buf_udp[8] == GET){
				reply_buf_udp[4] = 0x0A;
				reply_buf_udp[9] =  rcfm_status.rcfm_cal_en;
				*((uint64_t *)&reply_buf_udp[10]) = rcfm_status.rcfm_cal_freq;

				//RCV Packet
				memcpy(send_packet->payload, reply_buf_udp, p->len);
				send_packet->len = p -> len;
				send_packet->tot_len = p -> len;
			}
		}
		else if(recv_icd_header.CMD_CODE == 0x0160U){		//Set RF ANT Path
			if(reply_buf_udp[8] == SET){

			}
			else if(reply_buf_udp[8] == GET){
				reply_buf_udp[4] = 0x02;
				reply_buf_udp[9] =  rcfm_status.rcfm_ant_bias;

				//RCV Packet
				memcpy(send_packet->payload, reply_buf_udp, p->len);
				send_packet->len = p -> len;
				send_packet->tot_len = p -> len;
			}
		}
		else if(recv_icd_header.CMD_CODE == 0x0170U){		//Get RF Status
			if(reply_buf_udp[8] == SET){

			}
			else if(reply_buf_udp[8] == GET){
				GetStatusIBIT();

				reply_buf_udp[4] = 0x0B;
				reply_buf_udp[9] =  BIT_STATUS.LOCK_ADCLK;
				reply_buf_udp[10] =  BIT_STATUS.LOCK_BIT;
				reply_buf_udp[11] =  BIT_STATUS.LNA1;
				reply_buf_udp[12] =  BIT_STATUS.REF_SIG;
				reply_buf_udp[13] =  BIT_STATUS.RCFM_PWR;
				reply_buf_udp[14] =  BIT_STATUS.RCFM_INSERT;
				reply_buf_udp[15] =  BIT_STATUS.LNA2;
				reply_buf_udp[16] =  BIT_STATUS.LNA3;
				reply_buf_udp[17] =  BIT_STATUS.RCRM_PWR;
				reply_buf_udp[18] =  BIT_STATUS.RCRM_INSERT;

				//RCV Packet
				memcpy(send_packet->payload, reply_buf_udp, p->len + 10);
				send_packet->len = p -> len + 10;
				send_packet->tot_len = p -> len + 10;
			}
		}
		else if(recv_icd_header.CMD_CODE == 0x0180U){		//Get RF Log
			if(reply_buf_udp[8] == SET){

			}
			else if(reply_buf_udp[8] == GET){
				log_val = SPI_ReadReg(DPU_LOG, 0, 2);

				reply_buf_udp[4] = 0x03;
				reply_buf_udp[9] =  ((log_val >> 0U) & 0xFFU);
				reply_buf_udp[10] =  ((log_val >> 8U) & 0xFFU);

				//RCV Packet
				memcpy(send_packet->payload, reply_buf_udp, p->len + 2);
				send_packet->len = p -> len + 2;
				send_packet->tot_len = p -> len + 2;
			}
		}
		else if(recv_icd_header.CMD_CODE == 0x0190U){		//Get RF TMP
			if(reply_buf_udp[8] == SET){

			}
			else if(reply_buf_udp[8] == GET){
				reply_buf_udp[4] = 0x03;
				BIT_STATUS.RCFM_TMP = GetRFTmp(TMP_RCFM_DEV);
				BIT_STATUS.RCRM_TMP = GetRFTmp(TMP_RCRM_DEV);

				reply_buf_udp[9] = BIT_STATUS.RCFM_TMP;
				reply_buf_udp[10] = BIT_STATUS.RCRM_TMP;

				//RCV Packet
				memcpy(send_packet->payload, reply_buf_udp, p->len + 2);
				send_packet->len = p -> len + 2;
				send_packet->tot_len = p -> len + 2;
			}
		}
		else if(recv_icd_header.CMD_CODE == 0x0200U){		//Get RF LNA Mode
			if(reply_buf_udp[8] == SET){

			}
			else if(reply_buf_udp[8] == GET){
				reply_buf_udp[4] = 0x02;
				reply_buf_udp[9] = (uint8_t)(RF_STATUS.RCFM_LNA_MODE + RF_STATUS.RCRM_LNA_MODE);

				//RCV Packet
				memcpy(send_packet->payload, reply_buf_udp, p->len);
				send_packet->len = p -> len;
				send_packet->tot_len = p -> len;
			}
		}
		//Send RCV packet
		if(recv_icd_header.CMD_CODE != 0x0040U){
			error = udp_sendto(tpcb, send_packet, addr, port);
			if(error != ERR_OK){
				printf("== UDP RecvCallback : Error in udp_sendto(%d)\n", error);
			}
		}
	}

	pbuf_free(p);
	pbuf_free(send_packet);
	return;
}

static void StartApplication(void)
{
	err_t err;

	/* Create Server PCB */
	udp_pcb = udp_new();
	if (!udp_pcb) {
		xil_printf("UDP server: Error creating PCB. Out of Memory\r\n");
		return;
	}

	err = udp_bind(udp_pcb, IP_ADDR_ANY, UDP_CONN_PORT);
	if (err != ERR_OK) {
		xil_printf("UDP server: Unable to bind to port");
		xil_printf(" %d: err = %d\r\n", UDP_CONN_PORT, err);
		udp_remove(udp_pcb);
		return;
	}

	/* specify callback to use for incoming connections */
	udp_recv(udp_pcb, RecvCallback, NULL);

	return;
}

int TransferData(void)
{
	static uint8_t send_cnt = 0;
	static uint8_t last_flag = 0;
	err_t err;
	int send_size = UDP_SEND_BUFSIZE;

	struct pbuf *send_packet;

	//ICD Header
	//Command ID : 0x0041
	recv_buf_udp[0] = (uint8_t)(0x51);
	recv_buf_udp[1] = (uint8_t)(0xE1);
	recv_buf_udp[2] = (uint8_t)(0x41);
	recv_buf_udp[3] = (uint8_t)(0x00);
	memcpy(AddrSpecCurHeader, recv_buf_udp, 4);
	*((uint32_t *)AddrSpecCurHeader + 1U) = (uint32_t)((DPU_STATUS.SpecBin * 2U) + 0xCU);

	//ICD Body
	memcpy((AddrSpecCurHeader + 2), &DPU_STATUS, 12);



	if(DataReady == 1U){
		while(1){

			//UDP Send_Packet Allocation
			send_packet = pbuf_alloc(PBUF_TRANSPORT, UDP_SEND_BUFSIZE, PBUF_POOL);
			if(!send_packet){
				xil_printf("error allocating pbuf to send\r\n");
				return -1;
			}

			SendDone = 0;
			send_size = spec_packet_size - (UDP_SEND_BUFSIZE * send_cnt);

			if(send_size > UDP_SEND_BUFSIZE){
				send_size = UDP_SEND_BUFSIZE;
			}
			else{
				last_flag = 1;
			}

			memcpy(send_packet->payload, AddrSpecCurHeader, send_size);
			send_packet->tot_len = send_size;
			send_packet->len = send_size;
			err = udp_sendto(udp_pcb, send_packet, &client_ip, client_port);
			if (err != ERR_OK) {
				xil_printf("== UDP : Error on udp_sendto() %d\r\n", err);
				return err;
			}

			if(last_flag == 0U){
				send_cnt++;
				AddrSpecCurHeader += (UDP_SEND_BUFSIZE / 4U);
			}
			else{
				send_cnt = 0;
				SendDone = 1;
				last_flag = 0;
			}
			pbuf_free(send_packet);

			if(SendDone == 1U) {
				break;
			}
		}
	}
	return 0;
}

struct netif *UdpInitUser(void)
{
	struct netif *netif;
	unsigned char mac_ethernet_address[] = {
		0x00, 0x0a, 0x35, 0x00, 0x01, 0x71 };

	netif = &server_netif;

	init_platform();

	xil_printf("\r\n\r\n");
	xil_printf("=========== lwIP RAW Mode UDP Server Initialize ===========\n");

	lwip_init();

	/* Add network interface to the netif_list, and set it as default */
	if (!xemac_add(netif, NULL, NULL, NULL, mac_ethernet_address,
			PLATFORM_EMAC_BASEADDR)) {
		xil_printf("Error adding N/W interface\n\r");
		return NULL;
	}

	netif_set_default(netif);

	//Enable Interrupts
	platform_enable_interrupts();

	netif_set_up(netif);

	AssignDefaultIp(&(netif->ip_addr), &(netif->netmask), &(netif->gw));
	PrintIpSettings(&(netif->ip_addr), &(netif->netmask), &(netif->gw));
	xil_printf("\r\n");

	StartApplication();

	return netif;
}
