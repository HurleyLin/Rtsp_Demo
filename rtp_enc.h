/*************************************************************************
	File Name: rtp_enc.h
	Author: Hurley
	Mail: 11508118636@qq.com
	Created Time: Oct 13, 2018
 ************************************************************************/

#ifndef __RTP_ENC_H__
#define __RTP_ENC_H__

#include <stdint.h>

typedef struct __rtp_enc 
{
	uint32_t ssrc;
	uint32_t sample_rate;
	uint16_t seq;
	uint16_t pktsiz;
	uint16_t nbpkts;
	uint8_t  pt;
	uint8_t *szbuf;
} rtp_enc;

int rtp_enc_init (rtp_enc *e, uint16_t pktsiz, uint16_t nbpkts);
int rtp_enc_h264 (rtp_enc *e, const uint8_t *frame, int len, uint64_t ts, uint8_t *packets[], int pktsizs[]);
int rtp_enc_g711 (rtp_enc *e, const uint8_t *frame, int len, uint64_t ts, uint8_t *packets[], int pktsizs[]);
void rtp_enc_deinit (rtp_enc *e);

#endif

