//============================================================================
// Name        : rtp_nec.h
// Author      : Hurley
// Mail		   : 1150118636@qq.com
// Version     : 1.0.0
// Create On   : Oct 13, 2018
// Copyright   : Copyright (c) 2018 Hurley All rights reserved.
// Description : Hello World in C++, Ansi-style
//============================================================================

#ifndef RTP_ENC_H_
#define RTP_ENC_H_

#include <stdint.h>

typedef struct __rtp_enc
{
	uint32_t ssrc;
	uint32_t sample_rate;
	uint16_t seq;
	uint16_t pktsiz;
	uint16_t nbpkts;
	uint8_t pt;
	uint8_t *szbuf;
}rtp_enc;

int rtp_enc_init(rtp_enc *e,uint16_t pktsiz,uint16_t nbpkts);

int rtp_enc_h264(rtp_enc *e, const uint8_t *frame, int len, uint64_t ts, uint8_t *packets[], int pktsizs[]);

int rtp_enc_g711(rtp_enc *e, const uint8_t *frame, int len, uint64_t ts, uint8_t *packets[], int pktsizs[]);

void rtp_enc_deinit(rtp_enc *e);


#endif /* RTP_NEC_H_ */
