//============================================================================
// Name        : rtsp_demo.c
// Author      : Hurley
// Mail		   : 1150118636@qq.com
// Version     : 1.0.0
// Create On   : Oct 13, 2018
// Copyright   : Copyright (c) 2018 Hurley All rights reserved.
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

#include "comm.h"
#include "rtsp_demo.h"
#include "rtsp_msg.h"
#include "rtp_enc.h"
#include "queue.h"

struct resp_session;
struct stsp_client_connection;
TAILQ_HEAD(rtsp_session_queue_head,rtsp_session);
TAILQ_HEAD(rtsp_client_connection_queue_head,rtsp_client_connection);

struct rtsp_session
{
	char path[64];
	int has_video;
	int has_audio;
	uint8_t h264_sps[64];
	uint8_t h264_pps[64];
	int h264_sps_len;
	int h264_pps_len;

	rtp_enc vrtpe;
	rtp_enc artpe;

}


