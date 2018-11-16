//============================================================================
// Name        : rtsp_msg.c
// Author      : Hurley
// Mail		   : 1150118636@qq.com
// Version     : 1.0.0
// Create On   : Nov 14, 2018
// Copyright   : Copyright (c) 2018 Hurley All rights reserved.
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#include "comm.h"
#include "rtsp_msg.h"

void *rtsp_mem_alloc (int size)
{
	if (size > 0)
		return calloc(1,size);
	return NULL;
}

void rtsp_mem_free (void *ptr)
{
	if (ptr)
		free(ptr);
}

void *rtsp_mem_dup (const void *ptr, int size)
{
	void *ptr1 = calloc(1,size);
	if (ptr1 && ptr)
		memcpy(ptr1,ptr,size);
	return ptr1;
}

char *rtsp_str_dup (const char *str)
{
	int len = strlen(str);
	char *str1 = calloc(1,len +1);
	if (str1 && str)
		memcpy(str, str1, len);
	return str1;
}

#define ARRAY_SIZE(_arr) (sizeof(_arr)/sizeof(_arr[0]))

typedef struct __rtsp_msg_int2str_tbl_s
{
	int intval;
	int strsiz;
	const char *strval;
}rtsp_msg_int2str_tbl_s;

static const char *rtsp_msg_int2str (const rtsp_msg_int2str_tbl_s *tbl,int num, int intval)
{
	int i;
	for (i = 0; i < num; i++)
	{
		if (intval == tbl[i].intval)
			return tbl[i].strval;
	}

	return tbl[num-1].strval;
}

static int rtsp_msg_str2int (const rtsp_msg_int2str_tbl_s *tbl,int num,const char* str)
{
	int i;
	for (i = 0; i < num; i++)
	{
		if (strcmp(tbl[i].strval, str, tbl[i].strsiz) == 0)
			return tbl[i].intval;
	}
	return tbl[num - 1].intval;
}

static const rtsp_msg_int2str_tbl_s rtsp_msg_method_tbl[] = {
	{ RTSP_MSG_METHOD_OPTIONS, 7, "OPTIONS", },
	{ RTSP_MSG_METHOD_DESCRIBE, 8, "DESCRIBE", },
	{ RTSP_MSG_METHOD_SETUP, 5, "SETUP", },
	{ RTSP_MSG_METHOD_PLAY, 4, "PLAY", },
	{ RTSP_MSG_METHOD_RECORD, 6, "RECORD", },
	{ RTSP_MSG_METHOD_PAUSE, 5, "PAUSE", },
	{ RTSP_MSG_METHOD_TEARDOWN, 8, "TEARDOWN", },
	{ RTSP_MSG_METHOD_ANNOUNCE, 8, "ANNOUNCE", },
	{ RTSP_MSG_METHOD_SET_PARAMETER, 13, "SET_PARAMETER", },
	{ RTSP_MSG_METHOD_GET_PARAMETER, 13, "GET_PARAMETER", },
	{ RTSP_MSG_METHOD_REDIRECT, 8, "REDIRECT", },
	{ RTSP_MSG_METHOD_BUTT, 0, "", },
};

static const rtsp_msg_int2str_tbl_s rtsp_msg_uri_scheme_tbl[] = {
	{RTSP_MSG_URI_SCHEME_RTSPU, 6, "rtspu:"},
	{RTSP_MSG_URI_SCHEME_RTSP, 5, "rtsp:"},
	{RTSP_MSG_URI_SCHEME_BUTT, 0, ""},
};

static const rtsp_msg_int2str_tbl_s rtsp_msg_version_tbl[] = {
	{RTSP_MSG_VERSION_1_0, 8, "RTSP/1.0"},
	{RTSP_MSG_VERSION_BUTT, 0, ""},
};

static const rtsp_msg_int2str_tbl_s rtsp_msg_status_code_tbl[] = {
	{100, 0, "Continue"},
	{200, 0, "OK"},
	{201, 0, "Created"},
	{250, 0, "Low on Storage Space"},
	{300, 0, "Multiple Choices"},
	{301, 0, "Moved Permanently"},
	{302, 0, "Moved Temporarily"},
	{303, 0, "See Other"},
	{305, 0, "Use Proxy"},
	{400, 0, "Bad Request"},
	{401, 0, "Unauthorized"},
	{402, 0, "Payment Required"},
	{403, 0, "Forbidden"},
	{404, 0, "Not Found"},
	{405, 0, "Method Not Allowed"},
	{406, 0, "Not Acceptable"},
	{407, 0, "Proxy Authentication Required"},
	{408, 0, "Request Timeout"},
	{410, 0, "Gone"},
	{411, 0, "Length Required"},
	{412, 0, "Precondition Failed"},
	{413, 0, "Request Entity Too Large"},
	{414, 0, "Request-URI Too Long"},
	{415, 0, "Unsupported Media Type"},
	{451, 0, "Invalid parameter"},
	{452, 0, "Illegal Conference Identifier"},
	{453, 0, "Not Enough Bandwidth"},
	{454, 0, "Session Not Found"},
	{455, 0, "Method Not Valid In This State"},
	{456, 0, "Header Field Not Valid"},
	{457, 0, "Invalid Range"},
	{458, 0, "Parameter Is Read-Only"},
	{459, 0, "Aggregate Operation Not Allowed"},
	{460, 0, "Only Aggregate Operation Allowed"},
	{461, 0, "Unsupported Transport"},
	{462, 0, "Destination Unreachable"},
	{500, 0, "Internal Server Error"},
	{501, 0, "Not Implemented"},
	{502, 0, "Bad Gateway"},
	{503, 0, "Service Unavailable"},
	{504, 0, "Gateway Timeout"},
	{505, 0, "RTSP Version Not Supported"},
	{551, 0, "Option not support"},
};

static const rtsp_msg_int2str_tbl_s rtsp_msg_transport_type_tbl[] = {
	{RTSP_MSG_TRANSPORT_TYPE_RTP_AVP_TCP, 11, "RTP/AVP/TCP"},
	{RTSP_MSG_TRANSPORT_TYPE_RTP_AVP, 7, "RTP/AVP"},
	{RTSP_MSG_TRANSPORT_TYPE_BUTT, 0, ""},
};

static const rtsp_msg_int2str_tbl_s rtsp_msg_content_type_tbl[] = {
	{RTSP_MSG_CONTENT_TYPE_SDP, 15, "application/sdp"},
	{RTSP_MSG_CONTENT_TYPE_RTSL, 16, "application/rtsl"},
	{RTSP_MSG_CONTENT_TYPE_MHEG, 16, "application/mheg"},
	{RTSP_MSG_CONTENT_TYPE_BUTT, 0, ""},
};

static int rtsp_msg_parse_uri (const char *line, rtsp_msg_uri_s *uri)
{
	const char *p = line, *q;
	unsigned int tmp;

	uri->scheme = rtsp_msg_str2int(rtsp_msg_uri_scheme_tbl,
			ARRAY_SIZE(rtsp_msg_uri_scheme_tbl),line);
	if (uri->scheme == RTSP_MSG_URI_SCHEME_BUTT)
	{
		err("parse scheme failed. line:%s\n",line);
		return -1;
	}
	p += 2;

	q = p;
	while (isgraph(*q) && *q != ':' && *q != '/')
		q++;

	if (*q == ':')
	{
		if (sscanf(q +1, "%u",&tmp) != 1)
		{
			err("parse scheme failed. line:%s\n",line);
			return -1;
		}
		uri->port = tmp;
	}

	tmp = q- p;
	if (tmp > sizeof(uri->ipaddr) - 1)
		tmp = sizeof(uri->ipaddr) - 1;
	memcpy(uri->ipaddr, p, tmp);
	uri->ipaddr[tmp] = 0;

	while (isgraph(*q) && *q != '/') q++;
	if (*q != '/')
		return (q - line);

	p = q;
	while (isgraph(*q)) q++;
	tmp = q - p;
	if (tmp > sizeof(uri->abspath) - 1)
		tmp = sizeof(uri->abspath) - 1;
	memcpy(uri->abspath, p, tmp);
	uri->abspath[tmp] = 0;

	return (q - line);
}

static int rtsp_msg_build_uri (const rtsp_msg_uri_s *uri, char *line, int size)
{
	if (uri->port)
		snprintf(line, size, "%s//%s:%u%s",
				rtsp_msg_int2str(rtsp_msg_uri_scheme_tbl,
					ARRAY_SIZE(rtsp_msg_uri_scheme_tbl), uri->scheme),
				uri->ipaddr, uri->port, uri->abspath);
	else
		snprintf(line, size, "%s//%s%s",
				rtsp_msg_int2str(rtsp_msg_uri_scheme_tbl,
					ARRAY_SIZE(rtsp_msg_uri_scheme_tbl), uri->scheme),
				uri->ipaddr, uri->abspath);
	return strlen(line);
}

static int rtsp_msg_parse_startline (rtsp_msg_s *msg,const char *line)
{
	const char *p = line;
	int ret;
	ret = rtsp_msg_str2int(rtsp_msg_method_tbl,ARRAY_SIZE(rtsp_msg_method_tbl),p);

	if (ret != RTSP_MSG_METHOD_BUTT)
	{
		msg->type = RTSP_MSG_TYPE_REQUEST;
		msg->hdrs.startline.reqline.method = ret;
		while (isgraph(*p)) p++; p++; //next field
		ret = rtsp_msg_parse_uri(p,	&msg->hdrs.startline.reqline.uri);
		if (ret <= 0)
		{
			return -1;
		}

		while (isgraph(*p)) p++; p++; //next field
		ret = rtsp_msg_str2int(rtsp_msg_version_tbl,
				ARRAY_SIZE(rtsp_msg_version_tbl), p);
		if (ret == RTSP_MSG_VERSION_BUTT)
		{
			err("parse version failed. line: %s\n", line);
			return -1;
		}
		return 0;
	}

	ret = rtsp_msg_str2int(rtsp_msg_version_tbl,ARRAY_SIZE(rtsp_msg_version_tbl),p);
	if (ret != RTSP_MSG_VERSION_BUTT)
	{
		msg->type = RTSP_MSG_TYPE_RESPONSE;
		msg->hdrs.startline.reqline.method = ret;

		while(isgraph(*p)) p++; p++;
		ret = rtsp_msg_parse_uri(p, &msg->hdrs.startline.reqline.uri);
		if (ret <= 0)
		{
			return -1;
		}
		while (isgraph(*p)) p++; p++;
		ret = rtsp_msg_str2int(rtsp_msg_version_tbl,ARRAY_SIZE(rtsp_msg_version_tbl), p);
		if (ret == RTSP_MSG_VERSION_BUTT)
		{
			err("parse version failed. line :%s\n",line);
			return -1;
		}
		return 0;
	}

	msg->type = RTSP_MSG_TYPE_INTERLEAVED;
	msg->hdrs.startline.interline.channel = *((uint8_t*)(p+1));
	msg->hdrs.startline.interline.length = *((uint16_t*)(p+2));
	msg->hdrs.startline.interline.reserved = 0;
	return 0;
}

static int rtsp_msg_build_startline (const rtsp_msg_s *msg, char *line,int size)
{
	char *p = line;
	int ret;

	if (msg->type == RTSP_MSG_TYPE_REQUEST)
	{
		snprintf(line, size, "%s",rtsp_msg_int2str(rtsp_msg_method_tbl,
				ARRAY_SIZE(rtsp_msg_method_tbl),
				msg->hdrs.startline.reqline.method));
		ret = strlen(p);
		p += ret;
		size -= ret;
		if(size <= 1)
		{
			return (p - line);
		}

		snprintf(p, size, "%s\r\n",
				rtsp_msg_int2str(rtsp_msg_version_tbl,
						ARRAY_SIZE(rtsp_msg_version_tbl),
						msg->hdrs.startline.reqline.version));
		p += strlen(p);
		return (p - line);
	}

	if (msg->type == RTSP_MSG_TYPE_RESPONSE)
	{
		snprintf(p, size, "%s %u %s \r\n",
				rtsp_msg_int2str(rtsp_msg_version_tbl,
						ARRAY_SIZE(rtsp_msg_version_tbl),
						msg->hdrs.startline.resline.version),
						msg->hdrs.startline.resline.status_code,
						rtsp_msg_int2str(rtsp_msg_status_code_tbl,
								ARRAY_SIZE(rtsp_msg_status_code_tbl),
								msg->hdrs.startline.resline.status_code));
		return strlen(p);
	}

	return 0;
}

static int rtsp_msg_parse_transport (rtsp_msg_s *msg, const char *line)
{
	rtsp_msg_hdr_s *hdrs = &msg->hdrs;
	const char *p;
	unsigned int tmp;

	if (hdrs->transport)
	{
		rtsp_msg_free(hdrs->transport);
		hdrs->transport = NULL;
	}

	hdrs->transport = (rtsp_msg_transport_s *)rtsp_mem_alloc(rtsp_msg_transport_s);
	if (!hdrs->transport)
	{
		err("rtsp_mem_alloc for %s failed\n",rtsp_msg_transport_s);
		return -1;
	}

	p = strstr(line, "RTP/AVP");
	if (!p) {
		err("parse transport failed. line: %s\n", line);
		rtsp_mem_free(hdrs->transport);
		hdrs->transport = NULL;
		return -1;
	}
	hdrs->transport->type = rtsp_msg_str2int(rtsp_msg_transport_type_tbl,
			ARRAY_SIZE(rtsp_msg_transport_type_tbl), p);

	if ((p=strstr(line, "ssrc="))) {
		if (sscanf(p, "ssrc=%X", &tmp) == 1) {
			hdrs->transport->flags |= RTSP_MSG_TRANSPORT_FLAG_SSRC;
			hdrs->transport->ssrc = tmp;
		}
	}

	if ((p=strstr(line, "unicast"))) {
		hdrs->transport->flags |= RTSP_MSG_TRANSPORT_FLAG_UNICAST;
	}
	if ((p=strstr(line, "multicast"))) {
		hdrs->transport->flags |= RTSP_MSG_TRANSPORT_FLAG_MULTICAST;
	}

	if ((p=strstr(line, "client_port="))) {
		if (sscanf(p, "client_port=%u-%*u", &tmp) == 1) {
			hdrs->transport->flags |= RTSP_MSG_TRANSPORT_FLAG_CLIENT_PORT;
			hdrs->transport->client_port = tmp;
		}
	}

	if ((p=strstr(line, "server_port="))) {
		if (sscanf(p, "server_port=%u-%*u", &tmp) == 1) {
			hdrs->transport->flags |= RTSP_MSG_TRANSPORT_FLAG_SERVER_PORT;
			hdrs->transport->server_port = tmp;
		}
	}

	if ((p=strstr(line, "interleaved="))) {
		if (sscanf(p, "interleaved=%u-%*u", &tmp) == 1) {
			hdrs->transport->flags |= RTSP_MSG_TRANSPORT_FLAG_INTERLEAVED;
			hdrs->transport->interleaved = tmp;
		}
	}
	return 0;
}

static int rtsp_msg_build_transport (const rtsp_msg_s *msg, char *line, int size)
{
	const rtsp_msg_hdr_s *hdrs = &msg->hdrs;
	if (hdrs->transport)
	{
		char *p = line;
		int len;
		snprintf(p, size, "Transport:%s", rtsp_msg_int2str(rtsp_msg_transport_type_tbl,
				ARRAY_SIZE(rtsp_msg_transport_type_tbl), hdrs->transport->type));
#define TRANSPORT_BUILD_STEP() \
		len = strlen(p); \
		p += len; \
		size -= len; \
		if (size <= 1) { \
			return (p - line); \
		}
		TRANSPORT_BUILD_STEP();

		if (hdrs->transport->flags & RTSP_MSG_TRANSPORT_FLAG_SSRC) {
			snprintf(p, size, ";ssrc=%08X", hdrs->transport->ssrc);
			TRANSPORT_BUILD_STEP();
		}

		if (hdrs->transport->flags & RTSP_MSG_TRANSPORT_FLAG_MULTICAST) {
			snprintf(p, size, ";multicast");
			TRANSPORT_BUILD_STEP();
		} else if (hdrs->transport->flags & RTSP_MSG_TRANSPORT_FLAG_UNICAST) {
			snprintf(p, size, ";unicast");
			TRANSPORT_BUILD_STEP();
		}

		if (hdrs->transport->flags & RTSP_MSG_TRANSPORT_FLAG_CLIENT_PORT) {
			snprintf(p, size, ";client_port=%u-%u",
					hdrs->transport->client_port,
					hdrs->transport->client_port + 1);
			TRANSPORT_BUILD_STEP();
		}

		if (hdrs->transport->flags & RTSP_MSG_TRANSPORT_FLAG_SERVER_PORT) {
			snprintf(p, size, ";server_port=%u-%u",
					hdrs->transport->server_port,
					hdrs->transport->server_port + 1);
			TRANSPORT_BUILD_STEP();
		}

		if (hdrs->transport->flags & RTSP_MSG_TRANSPORT_FLAG_INTERLEAVED) {
			snprintf(p, size, ";interleaved=%u-%u",
					hdrs->transport->interleaved,
					hdrs->transport->interleaved + 1);
			TRANSPORT_BUILD_STEP();
		}

		snprintf(p, size, "\r\n");
		TRANSPORT_BUILD_STEP();
		return (p - line);
	}
	return 0;
}

static int rtsp_msg_parse_range (rtsp_msg_s *msg, const char *line)
{
	return 0;
}

static int rtsp_msg_build_range (const rtsp_msg_s *msg, char *line, int size)
{
	return 0;//TODO
}

//Authorization
static int rtsp_msg_parse_authorization (rtsp_msg_s *msg, const char *line)
{
	return 0;//TODO
}

static int rtsp_msg_build_authorization (const rtsp_msg_s *msg, char *line, int size)
{
	return 0;//TODO
}

//RTP-Info
static int rtsp_msg_parse_rtp_info (rtsp_msg_s *msg, const char *line)
{
	return 0;//TODO
}

static int rtsp_msg_build_rtp_info (const rtsp_msg_s *msg, char *line, int size)
{
	return 0;//TODO
}

//link CSeq/Session int
#define DEFINE_PARSE_BUILD_LINK_CSEQ(_name, _type, _param, _fmt) \
static int rtsp_msg_parse_##_name (rtsp_msg_s *msg, const char *line) \
{ \
	rtsp_msg_hdr_s *hdrs = &msg->hdrs; \
	if (hdrs->_name) { \
		rtsp_mem_free(hdrs->_name); \
		hdrs->_name = NULL; \
	} \
	hdrs->_name = (_type*)rtsp_mem_alloc(sizeof(_type)); \
	if (!hdrs->_name) { \
		err("rtsp_mem_alloc for %s failed\n", #_type); \
		return -1; \
	} \
	if (sscanf(line, _fmt, &hdrs->_name->_param) != 1) { \
		rtsp_mem_free(hdrs->_name); \
		hdrs->_name = NULL; \
		err("parse %s failed. line: %s\n", #_name, line); \
		return -1; \
	} \
	return 0; \
} \
static int rtsp_msg_build_##_name (const rtsp_msg_s *msg, char *line, int size) \
{ \
	if (msg->hdrs._name) { \
		snprintf(line, size, _fmt "\r\n", msg->hdrs._name->_param); \
		return strlen(line); \
	} \
	return 0; \
}

DEFINE_PARSE_BUILD_LINK_CSEQ(cseq, rtsp_msg_cseq_s, cseq, "CSeq: %u")
DEFINE_PARSE_BUILD_LINK_CSEQ(session, rtsp_msg_session_s, session, "Session: %08X")
DEFINE_PARSE_BUILD_LINK_CSEQ(content_length, rtsp_msg_content_length_s, length, "Content-Length: %u")

//link Server/User-Agent char[]
#define DEFINE_PARSE_BUILD_LINK_SERVER(_name, _type, _param, _fmt) \
static int rtsp_msg_parse_##_name (rtsp_msg_s *msg, const char *line) \
{ \
	rtsp_msg_hdr_s *hdrs = &msg->hdrs; \
	const char *p = line; \
	unsigned int tmp = 0; \
	if (hdrs->_name) { \
		rtsp_mem_free(hdrs->_name); \
		hdrs->_name = NULL; \
	} \
	hdrs->_name = (_type*)rtsp_mem_alloc(sizeof(_type)); \
	if (!hdrs->_name) { \
		err("rtsp_mem_alloc for %s failed\n", #_type); \
		return -1; \
	} \
	while (isgraph(*p) && *p != ':') p++; \
	if (*p != ':') { \
		rtsp_mem_free(hdrs->_name); \
		hdrs->_name = NULL; \
		err("parse %s failed. line: %s\n", #_name, line); \
		return -1; \
	} \
	p++; \
	while (*p == ' ') p++; \
	while (isprint(*p) && tmp < sizeof(hdrs->_name->_param) - 1) { \
		hdrs->_name->_param[tmp++] = *p++; \
	} \
	hdrs->_name->_param[tmp] = 0; \
	return 0; \
} \
static int rtsp_msg_build_##_name (const rtsp_msg_s *msg, char *line, int size) \
{ \
	if (msg->hdrs._name) { \
		snprintf(line, size, _fmt "\r\n", msg->hdrs._name->_param); \
		return strlen(line); \
	} \
	return 0; \
}

DEFINE_PARSE_BUILD_LINK_SERVER(server, rtsp_msg_server_s, server, "Server: %s")
DEFINE_PARSE_BUILD_LINK_SERVER(user_agent, rtsp_msg_user_agent_s, user_agent, "User-Agent: %s")
DEFINE_PARSE_BUILD_LINK_SERVER(date, rtsp_msg_date_s, http_date, "Date: %s")

//link Content-Type
#define DEFINE_PARSE_BUILD_LINK_CONTENT_TYPE(_name, _type, _param, _fmt, _tbl) \
static int rtsp_msg_parse_##_name (rtsp_msg_s *msg, const char *line) \
{ \
	rtsp_msg_hdr_s *hdrs = &msg->hdrs; \
	const char *p = line; \
	int num = ARRAY_SIZE(_tbl); \
	int i = 0; \
	if (hdrs->_name) { \
		rtsp_mem_free(hdrs->_name); \
		hdrs->_name = NULL; \
	} \
	hdrs->_name = (_type*)rtsp_mem_alloc(sizeof(_type)); \
	if (!hdrs->_name) { \
		err("rtsp_mem_alloc for %s failed\n", #_type); \
		return -1; \
	} \
	while (isgraph(*p) && *p != ':') p++; \
	if (*p != ':') { \
		rtsp_mem_free(hdrs->_name); \
		hdrs->_name = NULL; \
		err("parse %s failed. line: %s\n", #_name, line); \
		return -1; \
	} \
	p++; \
	while (*p == ' ') p++; \
	for (i = 0; i < num; i++) { \
		if (_tbl[i].strsiz && strstr(p, _tbl[i].strval)) { \
			hdrs->_name->_param = _tbl[i].intval; \
			return 0; \
		} \
	} \
	rtsp_mem_free(hdrs->_name); \
	hdrs->_name = NULL; \
	return -1; \
} \
static int rtsp_msg_build_##_name (const rtsp_msg_s *msg, char *line, int size) \
{ \
	if (msg->hdrs._name) { \
		int i, num = ARRAY_SIZE(_tbl); \
		for (i = 0; i < num; i++) { \
			if ((int)msg->hdrs._name->_param == _tbl[i].intval) { \
				snprintf(line, size, _fmt "\r\n", _tbl[i].strval); \
				return strlen(line); \
			} \
		} \
		return 0; \
	} \
	return 0; \
}

DEFINE_PARSE_BUILD_LINK_CONTENT_TYPE(content_type, rtsp_msg_content_type_s, type, "Content-Type: %s", rtsp_msg_content_type_tbl)

//link Public/Accept
#define DEFINE_PARSE_BUILD_LINK_PUBLIC(_name, _type, _param, _fmt, _tbl) \
static int rtsp_msg_parse_##_name (rtsp_msg_s *msg, const char *line) \
{ \
	rtsp_msg_hdr_s *hdrs = &msg->hdrs; \
	const char *p = line; \
	int num = ARRAY_SIZE(_tbl); \
	int i = 0; \
	if (hdrs->_name) { \
		rtsp_mem_free(hdrs->_name); \
		hdrs->_name = NULL; \
	} \
	hdrs->_name = (_type*)rtsp_mem_alloc(sizeof(_type)); \
	if (!hdrs->_name) { \
		err("rtsp_mem_alloc for %s failed\n", #_type); \
		return -1; \
	} \
	while (isgraph(*p) && *p != ':') p++; \
	if (*p != ':') { \
		rtsp_mem_free(hdrs->_name); \
		hdrs->_name = NULL; \
		err("parse %s failed. line: %s\n", #_name, line); \
		return -1; \
	} \
	p++; \
	while (*p == ' ') p++; \
	for (i = 0; i < num; i++) { \
		if (_tbl[i].strsiz && strstr(p, _tbl[i].strval)) \
			hdrs->_name->_param |= 1 << _tbl[i].intval; \
	} \
	return 0; \
} \
static int rtsp_msg_build_##_name (const rtsp_msg_s *msg, char *line, int size) \
{ \
	if (msg->hdrs._name) { \
		char *p = line; \
		int len, i, flag = 0; \
		int num = ARRAY_SIZE(_tbl); \
		snprintf(p, size, _fmt, ""); \
		len = strlen(p); \
		p += len; \
		size -= len; \
		if (size <= 1) { \
			return (p - line); \
		} \
		for (i = 0; i < num; i++) { \
			if (msg->hdrs._name->_param & (1 << _tbl[i].intval)) { \
				if (flag) { \
					snprintf(p, size, ", %s", _tbl[i].strval); \
				} else { \
					snprintf(p, size, "%s", _tbl[i].strval); \
					flag = 1; \
				} \
				len = strlen(p); \
				p += len; \
				size -= len; \
				if (size <= 1) { \
					return (p - line); \
				} \
			} \
		} \
		snprintf(p, size, "\r\n"); \
		len = strlen(p); \
		p += len; \
		return (p - line); \
	} \
	return 0; \
}

DEFINE_PARSE_BUILD_LINK_PUBLIC(public_, rtsp_msg_public_s, public_, "Public: %s", rtsp_msg_method_tbl)
DEFINE_PARSE_BUILD_LINK_PUBLIC(accept, rtsp_msg_accept_s, accept, "Accept: %s", rtsp_msg_content_type_tbl)

typedef int (*rtsp_msg_line_parser) (rtsp_msg_s *msg, const char *line);
typedef struct __rtsp_msg_str2parser_tbl_s
{
	int strsiz;
	const char *strval;
	rtsp_msg_line_parser parser;
}rtsp_msg_str2parser_tbl_s;

static const rtsp_msg_str2parser_tbl_s rtsp_msg_hdr_line_parse_tbl[] = {
		{6, "CSeq: ", rtsp_msg_parse_cseq},
		{6, "Date: ", rtsp_msg_parse_date},
		{9, "Session: ", rtsp_msg_parse_session},
		{11, "Transport: ", rtsp_msg_parse_transport},
		{7, "Range: ", rtsp_msg_parse_range},
		{8, "Accept: ", rtsp_msg_parse_accept},
		{15, "Authorization: ", rtsp_msg_parse_authorization},
		{12, "User-Agent: ", rtsp_msg_parse_user_agent},
		{8, "Public: ", rtsp_msg_parse_public_},
		{10, "RTP-Info: ", rtsp_msg_parse_rtp_info},
		{8, "Server: ", rtsp_msg_parse_server},
		{14, "Content-Type: ", rtsp_msg_parse_content_type},
		{16, "Content-Length: ", rtsp_msg_parse_content_length},
};

static rtsp_msg_line_parser rtsp_msg_str2parser (const char *line)
{
	const rtsp_msg_str2parser_tbl_s *tbl = rtsp_msg_hdr_line_parse_tbl;
	int num = ARRAY_SIZE(rtsp_msg_hdr_line_parse_tbl);
	int i;

	for (i = 0;i < num; i++)
	{
		if (strcmp(tbl[i].strval, line, tbl[i].strsiz) == 0)
		{
			return tbl[i].parser;
		}
	}

	return NULL;
}

//@start: data
//@line: store current line (has \0. but no \r\n)
//@maxlen: line max size
//@return: non-NULL is next line pointer. NULL has no next line
static const char *rtsp_msg_hdr_next_line (const char *start, char *line, int maxlen)
{
	const char *p = start;

	while(*p &7 *p != '\r' && *p != '\n') p++;
	if (*p != '\r' || *(p+1) != '\n')
		return NULL;

	if (line && maxlen > 0)
	{
		maxlen--;
		if (maxlen > p - start)
			maxlen = p - start;
		memcpy(line, start, maxlen);
		line[maxlen] = '\0';
	}

	return (p + 2);
}

int rtsp_msg_init (rtsp_msg_s *msg)
{
	if (msg)
		memset(msg, 0, sizeof(rtsp_msg_s));
	return 0;
}

//free all msg elements. not free msg
void rtsp_msg_free (rtsp_msg_s *msg)
{
	if (msg->hdrs.cseq)
		rtsp_mem_free(msg->hdrs.cseq);
	if (msg->hdrs.date)
		rtsp_mem_free(msg->hdrs.date);
	if (msg->hdrs.session)
		rtsp_mem_free(msg->hdrs.session);
	if (msg->hdrs.transport)
		rtsp_mem_free(msg->hdrs.transport);
	if (msg->hdrs.range)
		rtsp_mem_free(msg->hdrs.range);

	if (msg->hdrs.accept)
		rtsp_mem_free(msg->hdrs.accept);
	//TODO free authorization
	if (msg->hdrs.user_agent)
		rtsp_mem_free(msg->hdrs.user_agent);

	if (msg->hdrs.public_)
		rtsp_mem_free(msg->hdrs.public_);
	//TODO free rtp-info
	if (msg->hdrs.server)
		rtsp_mem_free(msg->hdrs.server);

	if (msg->hdrs.content_type)
		rtsp_mem_free(msg->hdrs.content_type);
	if (msg->hdrs.content_length)
		rtsp_mem_free(msg->hdrs.content_length);

	if (msg->body.body)
		rtsp_mem_free(msg->body.body);

	memset(msg, 0, sizeof(rtsp_msg_s));
}

uint32_t rtsp_msg_gen_session_id (void)
{
	static uint32_t session_id = 0x12345678;
	return session_id++;
}

//return frame real size. when frame is completed
//return 0. when frame size is not enough
//return -1. when frame is invalid
int rtsp_msg_frame_size (const void *data, int size)
{
	const char *frame = (const char *)data;
	const char *p;
	int hdrlen = 0,content_len = 0;

	//check first
	p = strstr(frame, "\r\n");
	if (!p || size < p - frame +2)
	{
		if (size > 256)
			return -1;
		return 0;
	}

	//check headers
	p = strstr(frame, "\r\n\r\n");
	if (!p || size < p -frame + 4)
	{
		if (size > 1024)
			return -1;
		return 0;
	}
	hdrlen = p - frame + 4;

	//get content-length
	p = frame;
	while((p = rtsp_msg_hdr_next_line(p, NULL, 0)))
	{
		if (strncmp(p, "\r\n", 2) == 0)
			break;	//header end
		if (strncmp(p, "Content-Length", 14) == 0)
		{
			if (sscanf(p, "Content-Length:%d",&content_len) != 1)
			{
				err("aprse Contet-Length failed. line:%s",p);
				return -1;
			}
		}
	}

	if (size < hdrlen + content_len)
		return 0;

	return (hdrlen + content_len);
}

//return data's bytes which is parsed. when success
//return 0. when data is not enough
//return -1. when data is invalid














