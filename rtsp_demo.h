//============================================================================
// Name        : rtsp_demo.h
// Author      : Hurley
// Mail		   : 1150118636@qq.com
// Version     : 1.0.0
// Create On   : Oct 13, 2018
// Copyright   : Copyright (c) 2018 Hurley All rights reserved.
// Description : Hello World in C++, Ansi-style
//============================================================================

#ifndef RTSP_DEMO_H_
#define RTSP_DEMO_H_

#include <stdint.h>
#include <sys/socket.h>

typedef void* rtsp_demo_handle;
typedef void* rtsp_session_handle;

rtsp_demo_handle rtsp_new_demo(int port);

rtsp_session_handle rtsp_new_session(rtsp_demo_handle demo, const char *path, int has_video, int has_audio);

int rtsp_do_event(rtsp_demo_handle demo);

int rtsp_tx_video(rtsp_session_handle session,const uint8_t *frame, int len, uint64_t ts);

int rtsp_tx_audio(rtsp_session_handle session, const uint8_t *frame, int len, uint64_t ts);

void rtsp_del_session(rtsp_session_handle session);

void rtsp_del_demo(rtsp_demo_handle demo);

#endif /* RTSP_DEMO_H_ */
