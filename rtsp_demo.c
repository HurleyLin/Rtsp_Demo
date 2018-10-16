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

	struct rtsp_demo *demo;
	struct rtsp_client_connection_queue_head connections_qhead;
	TAILQ_ENTRY(rtsp_session) demo_entry;
};

struct rtp_connection
{
	int is_over_tcp;
	int tcp_socket;
	int tcp_interleaved;
	int udp_sockfd[2];
	uint32_t ssrc;
};

struct rtsp_client_connection
{
	int state;
#define RTSP_CC_STATE_INIT			0;
#define RTSP_CC_STATE_READY			1;
#define RTSP_CC_STATE_PLAYING		2;
#define RTSP_CC_STATE_RECORDING		3;

	int sockfd;					//rtsp client socket
	unsigned long session_id;	//session id

	char reqbuf[1024];
	int reqlen;

	struct rtp_connection *vrtp;
	struct rtp_connection *vrtp;

	struct rtsp_demo *demo;
	struct rtsp_session *session;
	TAILQ_ENTRY(rtsp_client_connection) demo_entry;
	TAILQ_ENTRY(rtsp_client_connection) session_entry;

};


struct rtsp_demo
{
	int sockfd;		//rtsp server socket 0;invalid
	struct rtsp_session_queue_head sessions_qhead;
	struct rtsp_client_connection_queue_head connections_qhead;
};

static struct rtsp_demo *__alloc_demo (void)
{
	struct rtsp_demo *d = (struct rtsp_demo*) calloc(1, sizeof(struct rtsp_demo));
	if (NULL == d)　{
		err("alloc memory for rtsp_demo failed\n");
		return NULL;
	}
	TAILQ_INIT(&d->sessions_qhead);
	TAILQ_INIT(&d->connections_qhead);
	return d;
}

static void __free_demo (struct rtsp_demo *d)
{
	if (d)　{
		free(d);
	}
}

static struct rtsp_session *__alloc_session (struct rtsp_demo *d)
{
	struct rtsp_session *s = (struct rtsp_session*) calloc(1,sizeof(struct rtsp_session));
	if (NULL == s) {
		err("alloc memory for rtsp_session failed\n");
		return NULL;
	}

	s->demo = d;
	TAILQ_INIT(&s->connections_qhead);
	TAILQ_INSERT_TAIL(&d->sessions_qhead,s,demo_entry);
	return s;
}

static void __free_session (struct rtsp_session *s)
{
	if (s) {
		struct rtsp_demo *d = s->demo;
		TAILQ_REMOVE(&d->sessions_qhead, s, demo_entry);
		free(s);
	}
}

static struct rtsp_client_connection *__alloc_client_connection (struct rtsp_demo *d)
{
	struct rtsp_client_connection *cc = (struct rtsp_client_connection *)calloc(1, sizeof(struct rtsp_client_connection));
	if (NULL == cc) {

	}
	return cc;
}
