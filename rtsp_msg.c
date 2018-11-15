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
}


















