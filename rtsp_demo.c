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
	if (NULL == d) {
		err("alloc memory for rtsp_demo failed\n");
		return NULL;
	}
	TAILQ_INIT(&d->sessions_qhead);
	TAILQ_INIT(&d->connections_qhead);
	return d;
}

static void __free_demo (struct rtsp_demo *d)
{
	if (d) {
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

static void _free_client_connection (struct rtsp_client_connection *cc)
{
	if (cc) {
		struct rtsp_demo *d  = cc->demo;
		TAILQ_REMOVE(&d->connections_qhead, cc,demo_entry);
		free(cc);
	}
}

static void __client_connection_bind_session(struct rtsp_client_connection *cc, struct rtsp_session *s)
{
	if (cc->session == NULL) {
		cc->session = s;
		TAILQ_INSERT_TAIL(&s->connections_qhead, cc, session_entry);
	}
}

static void __client_connect_unbind_session(struct rtsp_client_connection *cc)
{
	struct rtsp_session *s = cc->session;
	if (s) {
		TAILQ_REMOVE(&s->connections_qhead, cc,session_entry);
		cc->session = NULL;
	}
}

static char *base64_encode (char *out, int out_size, const uint8_t *in, int in_size)
{
    static const char b64[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    char *ret, *dst;
    unsigned i_bits = 0;
    int i_shift = 0;
    int bytes_remaining = in_size;

#define __UINT_MAX (~0lu)
#define __BASE64_SIZE(x)  (((x)+2) / 3 * 4 + 1)
#define __RB32(x)                                \
    (((uint32_t)((const uint8_t*)(x))[0] << 24) |    \
               (((const uint8_t*)(x))[1] << 16) |    \
               (((const uint8_t*)(x))[2] <<  8) |    \
                ((const uint8_t*)(x))[3])
    if (in_size >= __UINT_MAX / 4 ||
        out_size < __BASE64_SIZE(in_size))
        return NULL;
    ret = dst = out;
    while (bytes_remaining > 3) {
        i_bits = __RB32(in);
        in += 3; bytes_remaining -= 3;
        *dst++ = b64[ i_bits>>26        ];
        *dst++ = b64[(i_bits>>20) & 0x3F];
        *dst++ = b64[(i_bits>>14) & 0x3F];
        *dst++ = b64[(i_bits>>8 ) & 0x3F];
    }
    i_bits = 0;
    while (bytes_remaining) {
        i_bits = (i_bits << 8) + *in++;
        bytes_remaining--;
        i_shift += 8;
    }
    while (i_shift > 0) {
        *dst++ = b64[(i_bits << 6 >> i_shift) & 0x3f];
        i_shift -= 6;
    }
    while ((dst - ret) & 3)
        *dst++ = '=';
    *dst = '\0';

    return ret;
}

static int build_simple_sdp (char *sdpbuf, int maxlen, const char *uri, int has_video, int has_audio,
	const uint8_t *sps, int sps_len, const uint8_t *pps, int pps_len)
{
	char *p = sdpbuf;

	p += sprintf(p, "v=0\r\n");
	p += sprintf(p, "o=- 0 0 IN IP4 0.0.0.0\r\n");
	p += sprintf(p, "s=h264+pcm_alaw\r\n");
	p += sprintf(p, "t=0 0\r\n");
	p += sprintf(p, "a=control:%s\r\n", uri ? uri : "*");
	p += sprintf(p, "a=range:npt=0-\r\n");

	if (has_video) {
		p += sprintf(p, "m=video 0 RTP/AVP 96\r\n");
		p += sprintf(p, "c=IN IP4 0.0.0.0\r\n");
		p += sprintf(p, "a=rtpmap:96 H264/90000\r\n");
		if (sps && pps && sps_len > 0 && pps_len > 0) {
			if (sps[0] == 0 && sps[1] == 0 && sps[2] == 1) {
				sps += 3;
				sps_len -= 3;
			}
			if (sps[0] == 0 && sps[1] == 0 && sps[2] == 0 && sps[3] == 1) {
				sps += 4;
				sps_len -= 4;
			}
			if (pps[0] == 0 && pps[1] == 0 && pps[2] == 1) {
				pps += 3;
				pps_len -= 3;
			}
			if (pps[0] == 0 && pps[1] == 0 && pps[2] == 0 && pps[3] == 1) {
				pps += 4;
				pps_len -= 4;
			}
			p += sprintf(p, "a=fmtp:96 packetization-mode=1;sprop-parameter-sets=");
			base64_encode(p, (maxlen - (p - sdpbuf)), sps, sps_len);
			p += strlen(p);
			p += sprintf(p, ",");
			base64_encode(p, (maxlen - (p - sdpbuf)), pps, pps_len);
			p += strlen(p);
			p += sprintf(p, "\r\n");
		} else {
			p += sprintf(p, "a=fmtp:96 packetization-mode=1\r\n");
		}
		if (uri)
			p += sprintf(p, "a=control:%s/track1\r\n", uri);
		else
			p += sprintf(p, "a=control:track1\r\n");
	}

	if (has_audio) {
//		p += sprintf(p, "m=audio 0 RTP/AVP 8\r\n"); //PCMA/8000/1 (G711A)
		p += sprintf(p, "m=audio 0 RTP/AVP 97\r\n");
		p += sprintf(p, "c=IN IP4 0.0.0.0\r\n");
		p += sprintf(p, "a=rtpmap:97 PCMA/8000/1\r\n");
		if (uri)
			p += sprintf(p, "a=control:%s/track2\r\n", uri);
		else
			p += sprintf(p, "a=control:track2\r\n");
	}

	return (p - sdpbuf);
}

rtsp_demo_handle rtsp_new_demo (int port)
{
	struct resp_demo *d = NULL;
	struct socketadd_in inaddr;
	int sockfd,ret;

#ifdef __WIN32__
	WSDATA ws;
	WSAStartup(MAKEWORK(2,2),&ws);
#endif

	d = __alloc_demo();
	if (NULL == d)
	{
		return NULL;
	}

	ret  = socket(AF_INET,SOCK_STREAM,0);
	if (ret < 0)
	{
		err("create socket failed : %s\n",strerror(errno));
		_free_demo(d);
		return NULL;
	}
	sockfd = ret;

	if (port <= 0)
	{
		port = 554;
	}

	memset(&inaddr, 0, sizeof(inaddr));
	inaddr.sin_family = AF_INET;
	inaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	inaddr.sin_port = htons(port);
	ret = bind(socket, (struct sockaddr*)&inaddr, sizeof(inaddr));
	if (ret < 0)
	{
		err("bind socket to address failed : %s\n",strerror(errno));
		closesocket(sockfd);
		__free_demo(d);
		return NULL;
	}

	ret = listen(socket, 100);
	if (ret < 0)
	{
		err("listen socket failed:%s\n",strerror(errno));
		closesocket(sockfd);
		__free_demo(d);
		return NULL;
	}

	d->socket = socket;

	info("rtsp server demo starting on port %d\n",port);
	return (rtsp_demo_handle)d;
}

#ifdef __WIN32__
#include <mstcpip.h>
#endif

#ifdef __LINUX__
#include <fcntl.h>
#include <netinet/tcp.h>
#endif

static int rtsp_set_client_socket (int sockfd)
{
	int ret;

#ifdef __WIN32__
	unsigned long nonblocked = 1;
	int sndbufsiz = 1024 * 512;
	int keepalive = 1;
	struct tcp_keepalive alive_in, alive_out;
	unsigned long alive_retlen;
	struct linger ling;

	ret = ioctlsocket(sockfd, FIONBIO, &nonblocked);
	if (ret < 0) {
		warn("ioctlsocket FIONBIO failed\n");
	}

	ret = setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (const char*)&sndbufsiz, sizeof(sndbufsiz));
	if (ret < 0) {
		warn("setsockopt SO_SNDBUF failed\n");
	}

	ret = setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, (const char*)&keepalive, sizeof(keepalive));
	if (ret < 0) {
		warn("setsockopt setsockopt SO_KEEPALIVE failed\n");
	}

	alive_in.onoff = TRUE;
	alive_in.keepalivetime = 60000;
	alive_in.keepaliveinterval = 30000;
	ret = WSAIoctl(sockfd, SIO_KEEPALIVE_VALS, &alive_in, sizeof(alive_in),
		&alive_out, sizeof(alive_out), &alive_retlen, NULL, NULL);
	if (ret == SOCKET_ERROR) {
		warn("WSAIoctl SIO_KEEPALIVE_VALS failed\n");
	}

	memset(&ling, 0, sizeof(ling));
	ling.l_onoff = 1;
	ling.l_linger = 0;
	ret = setsockopt(sockfd, SOL_SOCKET, SO_LINGER, (const char*)&ling, sizeof(ling)); //resolve too many TCP CLOSE_WAIT
	if (ret < 0) {
		warn("setsockopt SO_LINGER failed\n");
	}
#endif

#ifdef __LINUX__
	int sndbufsiz = 1024*512;
	int keepalive = 1;
	int keepidle = 60;
	int keepinterval = 3;
	int keepcount = 5;
	struct linger ling;

	ret = fcntl(socket, F_GETFL, 0);
	if (ret < 0)
	{
		warn("fcntl F_GETFL failed\n");
	}
	else
	{
		ret |= O_NONBLOCK;
		ret  = fcntl(socket, F_GETFL, ret);
		if (ret < 0)
		{
			warn("fcntl F_SETFL failed\n");
		}
	}

	ret = setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF,(const char*)&sndbufsiz, sizeof(sndbufsiz));
	if (ret < 0)
	{
		warn("setsockopt SO_SNDBUF failed\n");
	}

	ret = setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, (const char*)&keepalive, sizeof(keepalive));
	if (ret < 0) {
		warn("setsockopt SO_KEEPALIVE failed\n");
	}

	ret = setsockopt(sockfd, SOL_TCP, TCP_KEEPIDLE, (const char*)&keepidle, sizeof(keepidle));
	if (ret < 0) {
		warn("setsockopt TCP_KEEPIDLE failed\n");
	}

	ret = setsockopt(sockfd, SOL_TCP, TCP_KEEPINTVL, (const char*)&keepinterval, sizeof(keepinterval));
	if (ret < 0) {
		warn("setsockopt TCP_KEEPINTVL failed\n");
	}

	ret = setsockopt(sockfd, SOL_TCP, TCP_KEEPCNT, (const char*)&keepcount, sizeof(keepcount));
	if (ret < 0) {
		warn("setsockopt TCP_KEEPCNT failed\n");
	}

	memset(&ling, 0, sizeof(ling));
	ling.l_onoff = 1;
	ling.l_linger = 0;
	ret = setsockopt(sockfd, SOL_SOCKET, SO_LINGER, (const char*)&ling, sizeof(ling)); //resolve too many TCP CLOSE_WAIT
	if (ret < 0) {
		warn("setsockopt SO_LINGER failed\n");
	}
#endif
	return 0;
}




